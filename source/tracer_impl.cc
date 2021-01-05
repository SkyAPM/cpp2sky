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

#include "cpp2sky/exception.h"

namespace cpp2sky {

TracerImpl::TracerImpl(TracerConfig& config,
                       std::shared_ptr<grpc::ChannelCredentials> cred,
                       GrpcAsyncSegmentReporterStreamFactory& factory)
    : th_([this] { this->run(); }) {
  if (config.protocol() == Protocol::GRPC) {
    client_ = std::make_unique<GrpcAsyncSegmentReporterClient>(
        config.address(), config.token(), &cq_, factory, cred);
  } else {
    throw TracerException("REST is not supported.");
  }
}

TracerImpl::~TracerImpl() {
  client_.reset();
  th_.join();
  cq_.Shutdown();
}

void TracerImpl::sendSegment(SegmentContextPtr obj) {
  if (!obj) {
    return;
  }
  client_->sendMessage(obj->createSegmentObject());
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
    TaggedStream* t_stream = deTag(got_tag);
    if (!ok) {
      client_->resetStream();
      continue;
    }
    if (!t_stream->stream->handleOperation(t_stream->operation)) {
      client_->startStream();
    }
  }
}

TracerPtr createInsecureGrpcTracer(TracerConfig& cfg) {
  return std::make_unique<TracerImpl>(cfg, grpc::InsecureChannelCredentials(),
                                      stream_factory);
}

}  // namespace cpp2sky