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

 private:
  AsyncStreamFactory<StubType>& factory_;
  std::unique_ptr<StubType> stub_;
  grpc::CompletionQueue* cq_;
  grpc::ClientContext ctx_;
  std::shared_ptr<AsyncStream> stream_;
};

class GrpcAsyncSegmentReporterStream;

struct TaggedStream {
  enum class Operation : uint8_t {
    Init = 0,
    Write = 1,
    WriteDone = 2,
  };

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

 private:
  AsyncClient<StubType>* client_;
  Commands commands_;
  std::unique_ptr<grpc::ClientAsyncWriter<SegmentObject>> request_writer_;

  TaggedStream init_{TaggedStream::Operation::Init, this};
  TaggedStream write_{TaggedStream::Operation::Write, this};
  TaggedStream write_done_{TaggedStream::Operation::WriteDone, this};
};

class GrpcAsyncSegmentReporterStreamFactory final
    : public AsyncStreamFactory<StubType> {
 public:
  // AsyncStreamFactory
  AsyncStreamPtr create(AsyncClient<StubType>* client) override;
};

}  // namespace cpp2sky
