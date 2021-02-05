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
#include "language-agent/ConfigurationDiscoveryService.grpc.pb.h"
#include "language-agent/ConfigirationDiscoveryService.pb.h"

namespace cpp2sky {

using CdsRequest = skywalking::v3::ConfigurationSyncRequest;
using CdsResponse = skywalking::v3::Command;

class ConfigurationDiscoveryServiceStubImpl final : public ConfigurationDiscoveryServiceStub<CdsRequest, CdsResponse> {
public:
  ConfigurationDiscoveryServiceStubImpl(std::shared_ptr<grpc::Channel> channel);

  // ConfigurationDiscoveryServiceStub
  std::unique_ptr<grpc::ClientAsyncReader<CdsResponse>> createReader(
      grpc::ClientContext* ctx, CdsRequest* request, grpc::CompletionQueue* cq,
      void* tag) override;

private:
  std::unique_ptr<skywalking::v3::ConfigurationDiscoveryService> stub_;
};

}
