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

#pragma once

#include <thread>

#include "cpp2sky/tracer.h"
#include "source/grpc_async_client_impl.h"

namespace cpp2sky {

class TracerImpl : public Tracer {
 public:
  TracerImpl(std::string address,
             std::shared_ptr<grpc::ChannelCredentials> cred,
             GrpcAsyncSegmentReporterStreamFactory& factory);
  ~TracerImpl();

  void sendSegment(SegmentObject& obj) override { client_->sendMessage(obj); }
  void sendSegment(SegmentObject&& obj) override { client_->sendMessage(obj); }

 private:
  void run();

  GrpcAsyncSegmentReporterClient* client_;
  grpc::CompletionQueue cq_;
  std::thread th_;
};

static GrpcAsyncSegmentReporterStreamFactory stream_factory;

TracerPtr createInsecureGrpcTracer(std::string address);

}  // namespace cpp2sky
