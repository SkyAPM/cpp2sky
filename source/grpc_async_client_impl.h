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
#include <queue>

#include "cpp2sky/internal/async_client.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "language-agent/Tracing.pb.h"

namespace cpp2sky {

using StubType = TraceSegmentReportService::Stub;

class GrpcAsyncSegmentReporterClient final : public AsyncClient<StubType> {
 public:
  GrpcAsyncSegmentReporterClient(grpc::CompletionQueue* cq,
                                 AsyncStreamFactory<StubType>& factory,
                                 std::shared_ptr<grpc::ChannelCredentials> cred,
                                 std::string address);
  ~GrpcAsyncSegmentReporterClient();

  // AsyncClient
  bool sendMessage(Message& message) override;
  grpc::CompletionQueue* completionQueue() override { return cq_; }
  grpc::ClientContext* grpcClientContext() override { return &ctx_; }
  StubType* grpcStub() override { return stub_.get(); }
  std::string peerAddress() override { return address_; }

 private:
  std::string address_;
  AsyncStreamFactory<StubType>& factory_;
  std::unique_ptr<StubType> stub_;
  grpc::CompletionQueue* cq_;
  grpc::ClientContext ctx_;
  std::shared_ptr<AsyncStream> stream_;
};

class GrpcAsyncSegmentReporterStream;

struct TaggedStream {
  Operation operation;
  GrpcAsyncSegmentReporterStream* stream;
};

void* toTag(TaggedStream* stream);
TaggedStream* deTag(void* stream);

class GrpcAsyncSegmentReporterStream final : public AsyncStream {
 public:
  GrpcAsyncSegmentReporterStream(AsyncClient<StubType>* client);

  // AsyncStream
  bool startStream() override;
  bool sendMessage(Message& message) override;
  bool writeDone() override;
  Operation currentState() override { return state_; }
  void updateState(Operation op) override { state_ = op; }
  std::string peerAddress() override { return client_->peerAddress(); }

 private:
  AsyncClient<StubType>* client_;
  Commands commands_;
  std::unique_ptr<grpc::ClientAsyncWriter<SegmentObject>> request_writer_;
  std::queue<Message> pending_messages_;
  Operation state_{Operation::Initialized};

  TaggedStream connected_{Operation::Connected, this};
  TaggedStream write_{Operation::Write, this};
  TaggedStream write_done_{Operation::WriteDone, this};
};

class GrpcAsyncSegmentReporterStreamFactory final
    : public AsyncStreamFactory<StubType> {
 public:
  // AsyncStreamFactory
  AsyncStreamPtr create(AsyncClient<StubType>* client) override;
};

}  // namespace cpp2sky
