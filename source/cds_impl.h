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

namespace cpp2sky {

using CdsRequest = skywalking::v3::ConfigurationSyncRequest;
using CdsResponse = skywalking::v3::Commands;

class ConfigDiscoveryServiceStubImpl final
    : public StubWrapper<CdsRequest, CdsResponse> {
 public:
  ConfigDiscoveryServiceStubImpl(std::shared_ptr<grpc::Channel> channel);

  // StubWrapper
  std::unique_ptr<grpc::ClientAsyncWriter<CdsRequest>> createWriter(
      grpc::ClientContext* ctx, CdsResponse* response,
      grpc::CompletionQueue* cq, void* tag) override {
    assert(false);
  }

  std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>> createReader(
      grpc::ClientContext* ctx, CdsRequest* request,
      grpc::CompletionQueue* cq) override;

 private:
  std::unique_ptr<skywalking::v3::ConfigurationDiscoveryService::Stub> stub_;
};

class GrpcAsyncConfigDiscoveryServiceStream;

class GrpcAsyncConfigDiscoveryServiceClient final
    : public AsyncClient<CdsRequest, CdsResponse> {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceClient(
      const std::string& address, grpc::CompletionQueue& cq,
      UnaryStreamBuilderPtr<CdsRequest, CdsResponse> builder,
      std::shared_ptr<grpc::ChannelCredentials> cred);
  ~GrpcAsyncConfigDiscoveryServiceClient();

  void sendMessage(CdsRequest request);
  void drainPendingMessage(CdsRequest pending_message) override {}
  void startStream() override {}
  grpc::CompletionQueue& completionQueue() override { return cq_; }
  StubWrapper<CdsRequest, CdsResponse>& stub() override { return stub_; }

 private:
  void resetStream() {
    if (stream_) {
      gpr_log(GPR_INFO, "Stream %p had destroyed.", stream_.get());
      stream_.reset();
    }
  }

  std::string address_;
  UnaryStreamBuilderPtr<CdsRequest, CdsResponse> builder_;
  grpc::CompletionQueue& cq_;
  ConfigDiscoveryServiceStubImpl stub_;
  AsyncStreamPtr<CdsRequest, CdsResponse> stream_;
};

class GrpcAsyncConfigDiscoveryServiceStream final
    : public AsyncStream<CdsRequest, CdsResponse>,
      public AsyncStreamCallback {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceStream(
      AsyncClient<CdsRequest, CdsResponse>& parent, CdsRequest request);

  // AsyncStream
  void sendMessage(CdsRequest request);

  // AsyncStreamCallback
  void onReady() override {}
  void onIdle() override {}
  void onWriteDone() override {}
  void onReadDone() override;
  void onStreamFinish() override { delete this; }

 private:
  AsyncClient<CdsRequest, CdsResponse>& client_;
  std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>>
      response_reader_;
  CdsRequest request_;
  CdsResponse commands_;
  grpc::Status status_;
  grpc::ClientContext ctx_;
  StreamState state_{StreamState::Initialized};

  StreamCallbackTag read_done_{StreamState::ReadDone, this};
};

class GrpcAsyncConfigDiscoveryServiceStreamBuilder final
    : public UnaryStreamBuilder<CdsRequest, CdsResponse> {
 public:
  // AsyncStreamFactory
  AsyncStreamPtr<CdsRequest, CdsResponse> create(
      AsyncClient<CdsRequest, CdsResponse>& client,
      CdsRequest request) override;
};

}  // namespace cpp2sky
