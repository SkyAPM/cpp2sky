// Copyright 2020 SkyAPM

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/tracer_impl.h"

#include <chrono>
#include <thread>

#include "cds_impl.h"
#include "cpp2sky/exception.h"
#include "language-agent/ConfigurationDiscoveryService.pb.h"
#include "matchers/suffix_matcher.h"
#include "spdlog/spdlog.h"

namespace cpp2sky {

TracerImpl::TracerImpl(TracerConfig& config,
                       std::shared_ptr<grpc::ChannelCredentials> cred)
    : config_(config),
      evloop_thread_([this] { this->run(); }),
      segment_factory_(config) {
  init(config, cred);
}

TracerImpl::TracerImpl(
    TracerConfig& config,
    AsyncClientPtr<TracerRequestType, TracerResponseType> reporter_client)
    : config_(config),
      reporter_client_(std::move(reporter_client)),
      evloop_thread_([this] { this->run(); }),
      segment_factory_(config) {
  init(config, nullptr);
}

TracerImpl::~TracerImpl() {
  reporter_client_.reset();
  cds_client_.reset();
  cq_.Shutdown();
  evloop_thread_.join();
}

TracingContextPtr TracerImpl::newContext() { return segment_factory_.create(); }

TracingContextPtr TracerImpl::newContext(SpanContextPtr span) {
  return segment_factory_.create(span);
}

bool TracerImpl::report(TracingContextPtr obj) {
  if (!obj || !obj->readyToSend()) {
    return false;
  }

  for (const auto& op_name_matcher : op_name_matchers_) {
    if (!obj->spans().empty() &&
        op_name_matcher->match(obj->spans().front()->operationName())) {
      return false;
    }
  }

  reporter_client_->sendMessage(obj->createSegmentObject());
  return true;
}

void TracerImpl::run() {
  void* got_tag;
  bool ok = false;
  while (true) {
    // TODO(shikugawa): cleanup evloop handler.
    if (cds_timer_ != nullptr && cds_timer_->check()) {
      cdsRequest();
    }

    grpc::CompletionQueue::NextStatus status = cq_.AsyncNext(
        &got_tag, &ok, gpr_time_from_nanos(0, GPR_CLOCK_REALTIME));
    switch (status) {
      case grpc::CompletionQueue::TIMEOUT:
        continue;
      case grpc::CompletionQueue::SHUTDOWN:
        return;
    }
    static_cast<StreamCallbackTag*>(got_tag)->callback(!ok);
  }
}

void TracerImpl::cdsRequest() {
  skywalking::v3::ConfigurationSyncRequest request;
  request.set_service(config_.tracerConfig().service_name());
  request.set_uuid(config_.uuid());
  cds_client_->sendMessage(request);
}

void TracerImpl::init(TracerConfig& config,
                      std::shared_ptr<grpc::ChannelCredentials> cred) {
  spdlog::set_level(spdlog::level::warn);

  if (reporter_client_ == nullptr) {
    if (config.protocol() == Protocol::GRPC) {
      reporter_client_ = std::make_unique<GrpcAsyncSegmentReporterClient>(
          config.address(), cq_,
          std::make_unique<GrpcAsyncSegmentReporterStreamBuilder>(
              config.token()),
          cred);
    } else {
      throw TracerException("REST is not supported.");
    }
  }

  op_name_matchers_.emplace_back(std::make_unique<SuffixMatcher>(
      std::vector<std::string>(config.ignore_operation_name_suffix().begin(),
                               config.ignore_operation_name_suffix().end())));

  if (config_.tracerConfig().cds_request_interval() != 0) {
    cds_client_ = std::make_unique<GrpcAsyncConfigDiscoveryServiceClient>(
        config.address(), cq_,
        std::make_unique<GrpcAsyncConfigDiscoveryServiceStreamBuilder>(config_),
        cred);
    cds_timer_ =
        std::make_unique<Timer>(config_.tracerConfig().cds_request_interval());
  }
}

TracerPtr createInsecureGrpcTracer(TracerConfig& cfg) {
  return std::make_unique<TracerImpl>(cfg, grpc::InsecureChannelCredentials());
}

}  // namespace cpp2sky
