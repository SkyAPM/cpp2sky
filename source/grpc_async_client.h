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

#include <grpcpp/grpcpp.h>

#include <memory>

#include "cpp2sky/async_client.h"
#include "language-agent/Tracing.grpc.pb.h"

namespace cpp2sky {

class GrpcAsyncSegmentReporterClient : public AsyncClient {
 public:
  GrpcAsyncSegmentReporterClient(grpc::CompletionQueue& cq,
                                 std::shared_ptr<grpc::Channel> channel);

 private:
  grpc::CompletionQueue& cq_;
  TraceSegmentReportService::Stub stub_;
};

}  // namespace cpp2sky
