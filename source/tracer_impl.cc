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

#include "source/utils/exception.h"

namespace cpp2sky {

TracerImpl::TracerImpl(std::string address,
                       std::shared_ptr<grpc::ChannelCredentials> cred,
                       GrpcAsyncSegmentReporterStreamFactory& factory)
    : th_([this] { this->run(); }) {
  client_ = new GrpcAsyncSegmentReporterClient(&cq_, factory, cred, address);
}

TracerImpl::~TracerImpl() {
  delete client_;
  th_.join();
  cq_.Shutdown();
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
      continue;
    }
    if (!t_stream->stream->handleOperation(t_stream->operation)) {
      return;
    }
  }
}

TracerPtr createInsecureGrpcTracer(std::string address) {
  return std::make_unique<TracerImpl>(
      address, grpc::InsecureChannelCredentials(), stream_factory);
}

}  // namespace cpp2sky
