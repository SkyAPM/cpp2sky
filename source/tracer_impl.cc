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
#include "spdlog/spdlog.h"

namespace cpp2sky {

TracerImpl::TracerImpl(TracerConfig& config,
                       std::shared_ptr<grpc::ChannelCredentials> cred)
    : config_(config),
      grpc_callback_thread_([this] { this->run(); }),
      segment_factory_(config_) {
  // spdlog::set_level(spdlog::level::warn);

  if (config.protocol() == Protocol::GRPC) {
    reporter_client_ = std::make_unique<GrpcAsyncSegmentReporterClient>(
        config.address(), cq_,
        std::make_unique<GrpcAsyncSegmentReporterStreamBuilder>(config.token()),
        cred);
  } else {
    throw TracerException("REST is not supported.");
  }

  if (config_.tracerConfig().cds_request_interval() != 0) {
    cds_client_ = std::make_unique<GrpcAsyncConfigDiscoveryServiceClient>(
        config.address(), cq_,
        std::make_unique<GrpcAsyncConfigDiscoveryServiceStreamBuilder>(config_),
        cred);
    cds_thread_ = std::thread([this] {
      this->startCds(
          std::chrono::seconds(config_.tracerConfig().cds_request_interval()));
    });
  }
}

TracerImpl::~TracerImpl() {
  reporter_client_.reset();
  cds_client_.reset();
  cq_.Shutdown();
  grpc_callback_thread_.join();

  if (cds_thread_.joinable()) {
    cds_thread_.join();
  }
}

TracingContextPtr TracerImpl::newContext() { return segment_factory_.create(); }

TracingContextPtr TracerImpl::newContext(SpanContextPtr span) {
  return segment_factory_.create(span);
}

void TracerImpl::report(TracingContextPtr obj) {
  if (!obj || !obj->readyToSend()) {
    return;
  }
  reporter_client_->sendMessage(obj->createSegmentObject());
}

void TracerImpl::run() {
  void* got_tag;
  bool ok = false;
  while (true) {
    grpc::CompletionQueue::NextStatus status =
        cq_.AsyncNext(&got_tag, &ok, gpr_inf_future(GPR_CLOCK_REALTIME));
    if (status == grpc::CompletionQueue::SHUTDOWN) {
      return;
    }
    static_cast<StreamCallbackTag*>(got_tag)->callback(!ok);
  }
}

void TracerImpl::startCds(std::chrono::seconds seconds) {
  while (true) {
    skywalking::v3::ConfigurationSyncRequest request;
    request.set_service(config_.tracerConfig().service_name());
    request.set_uuid(config_.uuid());
    cds_client_->sendMessage(request);
    std::this_thread::sleep_for(seconds);
  }
}

TracerPtr createInsecureGrpcTracer(TracerConfig& cfg) {
  return std::make_unique<TracerImpl>(cfg, grpc::InsecureChannelCredentials());
}

}  // namespace cpp2sky
