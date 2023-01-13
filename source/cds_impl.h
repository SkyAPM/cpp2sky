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
#include "cpp2sky/internal/stream_builder.h"
#include "language-agent/ConfigurationDiscoveryService.grpc.pb.h"
#include "language-agent/ConfigurationDiscoveryService.pb.h"
#include "source/dynamic_config.h"

namespace cpp2sky {

using CdsRequest = skywalking::v3::ConfigurationSyncRequest;
using CdsResponse = skywalking::v3::Commands;

class GrpcAsyncConfigDiscoveryServiceStream;

class GrpcAsyncConfigDiscoveryServiceClient final
    : public AsyncClient<CdsRequest, CdsResponse> {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceClient(
      const std::string& address, grpc::CompletionQueue& cq,
      UnaryStreamBuilderPtr<CdsRequest, CdsResponse> builder,
      std::shared_ptr<grpc::ChannelCredentials> cred);
  ~GrpcAsyncConfigDiscoveryServiceClient();

  void sendMessage(CdsRequest request) override;
  void startStream() override {}
  grpc::CompletionQueue& completionQueue() override { return cq_; }
  grpc::TemplatedGenericStub<CdsRequest, CdsResponse>& stub() override {
    return stub_;
  }

 private:
  void resetStream();

  std::string address_;
  UnaryStreamBuilderPtr<CdsRequest, CdsResponse> builder_;
  grpc::CompletionQueue& cq_;
  grpc::TemplatedGenericStub<CdsRequest, CdsResponse> stub_;
  AsyncStreamSharedPtr<CdsRequest, CdsResponse> stream_;
};

class GrpcAsyncConfigDiscoveryServiceStream final
    : public AsyncStream<CdsRequest, CdsResponse> {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceStream(
      AsyncClient<CdsRequest, CdsResponse>& parent, DynamicConfig& config);

  // AsyncStream
  void sendMessage(CdsRequest request) override;

 private:
  AsyncClient<CdsRequest, CdsResponse>& client_;
  std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>>
      response_reader_;

  CdsResponse commands_;
  grpc::Status status_;
  grpc::ClientContext ctx_;
  DynamicConfig& config_;

  StreamCallbackTag read_tag_;
};

class GrpcAsyncConfigDiscoveryServiceStreamBuilder final
    : public UnaryStreamBuilder<CdsRequest, CdsResponse> {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceStreamBuilder(DynamicConfig& config)
      : config_(config) {}

  // AsyncStreamFactory
  AsyncStreamSharedPtr<CdsRequest, CdsResponse> create(
      AsyncClient<CdsRequest, CdsResponse>& client) override;

 private:
  DynamicConfig& config_;
};

}  // namespace cpp2sky
