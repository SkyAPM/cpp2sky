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
    : public StubWrapper<CdsRequest, CdsResponse> {
 public:
  ConfigDiscoveryServiceStubImpl(std::shared_ptr<grpc::Channel> channel);

  // StubWrapper
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> createWriter(
      grpc::ClientContext* ctx, TracerResponseType* response,
      grpc::CompletionQueue* cq, void* tag) override {
    assert(false);
  }

  std::unique_ptr<grpc::ClientAsyncResponseReader<TracerResponseType>>
  createReader(grpc::ClientContext* ctx, TracerRequestType* request,
               grpc::CompletionQueue* cq) override;

 private:
  std::unique_ptr<skywalking::v3::ConfigurationDiscoveryService> stub_;
};

class GrpcAsyncConfigDiscoveryServiceStream;

class GrpcAsyncConfigDiscoveryServiceClient final
    : public AsyncClient<CdsRequest, CdsResponse> {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceClient(
      const std::string& address, grpc::CompletionQueue& cq,
      AsyncStreamFactoryPtr<CdsRequest, CdsResponse> factory,
      std::shared_ptr<grpc::ChannelCredentials> cred);
  ~GrpcAsyncConfigDiscoveryServiceClient();

  void sendMessage(CdsRequest request);
  std::string peerAddress() override { return address_; }
  void drainPendingMessage(TracerRequestType pending_message) override {}
  void startStream() override;
  size_t numOfMessages() override { assert(false); }
  grpc::CompletionQueue& completionQueue() override { return cq_; }
  StubWrapper<RequestType, ResponseType>& stub() override { return stub_; }

 private:
  void resetStream() {
    if (stream_) {
      gpr_log(GPR_INFO, "Stream %p had destroyed.", stream_.get());
      stream_.reset();
    }
  }

  std::string address_;
  AsyncStreamFactoryPtr<TracerRequestType, TracerResponseType> factory_;
  grpc::CompletionQueue& cq_;
  ConfigDiscoveryServiceStubImpl stub_;
  AsyncStreamPtr<CdsRequest, CdsResponse> stream_;
};

class GrpcAsyncConfigDiscoveryServiceStream final
    : public AsyncStream<CdsRequest, CdsResponse>,
      public AsyncStreamCallback {
 public:
  explicit GrpcAsyncConfigDiscoveryServiceStream(
      AsyncClient<CdsRequest, CdsResponse>& parent);

  // AsyncStream
  void sendMessage(CdsRequest request);

  // AsyncStreamCallback
  void onConnected() override {}
  void onIdle() override {}
  void onWriteDone() override {}
  void onReadDone() override;

 private:
  AsyncClient<CdsRequest, CdsResponse>& client_;
  std::unique_ptr<grpc::ClientAsyncResponseReader<TracerResponseType>> response_reader_;
  CdsRequest request_;
  CdsResponse commands_;
  grpc::Status status_;
  grpc::ClientContext ctx_;
  StreamState state_{StreamState::Initialized};

  StreamCallbackTag read_done_{StreamState::ReadDone, this};
};

class GrpcAsyncConfigDiscoveryServiceStreamFactory final
  : public AsyncStreamFactory<CdsRequest, CdsResponse> {
public:
  // AsyncStreamFactory
  AsyncStreamPtr<CdsRequest, CdsResponse> create(
    AsyncClient<TracerRequestType, TracerResponseType>& client,
    std::condition_variable& cv) override;
};

}  // namespace cpp2sky
