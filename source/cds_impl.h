// Copyright 2021 SkyAPM

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

#include "cpp2sky/internal/async_client.h"
#include "language-agent/ConfigirationDiscoveryService.pb.h"
#include "language-agent/ConfigurationDiscoveryService.grpc.pb.h"

namespace cpp2sky {

using CdsRequest = skywalking::v3::ConfigurationSyncRequest;
using CdsResponse = skywalking::v3::Command;

class ConfigDiscoveryServiceStubImpl final
    : public ConfigDiscoveryServiceStub<CdsRequest, CdsResponse> {
 public:
  ConfigDiscoveryServiceStubImpl(std::shared_ptr<grpc::Channel> channel);

  // ConfigDiscoveryServiceStub
  std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>> createReader(
      grpc::ClientContext* ctx, CdsRequest* request,
      grpc::CompletionQueue* cq) override;

 private:
  std::unique_ptr<skywalking::v3::ConfigurationDiscoveryService> stub_;
};

using ConfigDiscoveryServiceStubPtr =
    std::unique_ptr<ConfigDiscoveryServiceStub<CdsRequest, CdsResponse>>;

class GrpcAsyncConfigDiscoveryServiceStream;

using GrpcAsyncConfigDiscoveryServiceStreamPtr =
    std::unique_ptr<GrpcAsyncConfigDiscoveryServiceStream>;

class GrpcAsyncConfigDiscoveryServiceClient {
 public:
  GrpcAsyncConfigDiscoveryServiceClient(
      const std::string& address, grpc::CompletionQueue* cq,
      std::shared_ptr<grpc::ChannelCredentials> cred);

  void sendMessage(CdsRequest request);

 private:
  void resetStream() {
    if (stream_) {
      gpr_log(GPR_INFO, "Stream %p had destroyed.", stream_.get());
      stream_.reset();
    }
  }

  std::string token_;
  std::string address_;
  ConfigDiscoveryServiceStubPtr stub_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::Channel> channel_;

  GrpcAsyncConfigDiscoveryServiceStreamPtr stream_;
};

struct TaggedStream2 {
  Operation2 operation;
  GrpcAsyncConfigDiscoveryServiceStream* stream;
};

class GrpcAsyncConfigDiscoveryServiceStream {
 public:
  GrpcAsyncConfigDiscoveryServiceStream(
      GrpcAsyncConfigDiscoveryServiceClient& parent);

  void sendMessage(CdsRequest request);

 private:
  GrpcAsyncConfigDiscoveryServiceClient& client_;
  CdsResponse commands_;
  grpc::ClientContext ctx_;
  Operation2 state_{Operation::Initialized};

  TaggedStream2 initialized_{Operation2::Initialized, this};
  TaggedStream2 write_done_{Operation2::WriteDone, this};
};

}  // namespace cpp2sky
