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

#include <list>
#include <memory>

#include "cpp2sky/internal/async_client.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "language-agent/Tracing.pb.h"

namespace cpp2sky {

using StubType = TraceSegmentReportService::Stub;

class GrpcAsyncSegmentReporterClient final : public AsyncClient<StubType> {
 public:
  GrpcAsyncSegmentReporterClient(grpc::CompletionQueue& cq,
                                 std::shared_ptr<grpc::Channel> channel,
                                 AsyncStreamFactory<StubType>& factory);

  // AsyncClient
  void onSendMessage(const Message& message) override;
  size_t numOfStreams() const override { return streams_.size(); }
  grpc::CompletionQueue& grpcDispatchQueue() override { return cq_; }
  grpc::ClientContext& grpcClientContext() override { return ctx_; }
  StubType& grpcStub() override { return stub_; }

 private:
  grpc::CompletionQueue& cq_;
  StubType stub_;
  grpc::ClientContext ctx_;
  std::list<AsyncStreamPtr> streams_;
  AsyncStreamFactory<StubType>& factory_;
};

class GrpcAsyncSegmentReporterStream final : public AsyncStream {
 public:
  GrpcAsyncSegmentReporterStream(AsyncClient<StubType>* parent)
      : parent_(parent) {}

  // AsyncStream
  uint16_t status() const override;
  void startStream() override;
  void setData(const Message& message) override;
  const Message& reply() const override;

 private:
  bool data_set_{false};
  SegmentObject data_;
  AsyncClient<StubType>* parent_;
  grpc::Status status_;
  Commands commands_;
  std::unique_ptr<grpc::ClientAsyncWriter<SegmentObject>> request_writer_;
};

class GrpcAsyncSegmentReporterStreamFactory final
    : public AsyncStreamFactory<StubType> {
 public:
  // AsyncStreamFactory
  AsyncStreamPtr create(AsyncClient<StubType>* client) override;
};

}  // namespace cpp2sky
