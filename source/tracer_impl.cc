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

#include <iostream>

namespace cpp2sky {

TracerImpl::TracerImpl(std::string address,
                       std::shared_ptr<grpc::ChannelCredentials> cred,
                       GrpcAsyncSegmentReporterStreamFactory& factory)
    : th_([this] { this->run(); }) {
  client_ = new GrpcAsyncSegmentReporterClient(&cq_, factory, cred, address);
}

TracerImpl::~TracerImpl() {
  delete client_;
  cq_.Shutdown();
  th_.join();
}

void TracerImpl::run() {
  void* got_tag;
  bool ok = false;
  while (cq_.Next(&got_tag, &ok)) {
    if (!ok) {
      continue;
    }
    TaggedStream* t_stream = deTag(got_tag);
    if (t_stream->operation == TaggedStream::Operation::Init) {
      std::cout << "Connected" << std::endl;
    } else if (t_stream->operation == TaggedStream::Operation::Write) {
      std::cout << "Write finished" << std::endl;
    } else if (t_stream->operation == TaggedStream::Operation::WriteDone) {
      std::cout << "Write done, this stream will be closed" << std::endl;
    } else {
      GPR_ASSERT(false);
    }
  }
}

TracerPtr createInsecureGrpcTracer(std::string address) {
  return std::make_unique<TracerImpl>(
      address, grpc::InsecureChannelCredentials(), stream_factory);
}

}  // namespace cpp2sky
