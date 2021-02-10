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

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "cpp2sky/config.pb.h"
#include "cpp2sky/internal/async_client.h"
#include "cpp2sky/internal/stream_builder.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "language-agent/Tracing.pb.h"
#include "source/utils/circular_buffer.h"

#define DRAIN_BUFFER_SIZE 1024
#define PENDING_MESSAGE_BUFFER_SIZE 1024

namespace cpp2sky {

using TracerRequestType = skywalking::v3::SegmentObject;
using TracerResponseType = skywalking::v3::Commands;

class TracerStubImpl final
    : public StubWrapper<TracerRequestType, TracerResponseType> {
 public:
  TracerStubImpl(std::shared_ptr<grpc::Channel> channel);

  // StubWrapper
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> createWriter(
      grpc::ClientContext* ctx, TracerResponseType* response,
      grpc::CompletionQueue* cq, void* tag) override;

  std::unique_ptr<grpc::ClientAsyncResponseReader<TracerResponseType>>
  createReader(grpc::ClientContext* ctx, TracerRequestType* request,
               grpc::CompletionQueue* cq) override {
    assert(false);
  }

 private:
  std::unique_ptr<skywalking::v3::TraceSegmentReportService::Stub> stub_;
};

class GrpcAsyncSegmentReporterStream;

class GrpcAsyncSegmentReporterClient final
    : public AsyncClient<TracerRequestType, TracerResponseType> {
 public:
  GrpcAsyncSegmentReporterClient(
      const std::string& address, grpc::CompletionQueue& cq,
      ClientStreamingStreamBuilderPtr<TracerRequestType, TracerResponseType>
          factory,
      std::shared_ptr<grpc::ChannelCredentials> cred);
  ~GrpcAsyncSegmentReporterClient();

  // AsyncClient
  void sendMessage(TracerRequestType message) override;
  void drainPendingMessage(TracerRequestType pending_message) override {
    drained_messages_.push(pending_message);
  }
  void startStream() override;
  StubWrapper<TracerRequestType, TracerResponseType>& stub() override {
    return stub_;
  }
  grpc::CompletionQueue& completionQueue() override { return cq_; }

  size_t numOfMessages() { return drained_messages_.size(); }

 private:
  void resetStream() {
    if (stream_) {
      gpr_log(GPR_INFO, "Stream %p had destroyed.", stream_.get());
      stream_.reset();
    }
  }

  std::string address_;
  ClientStreamingStreamBuilderPtr<TracerRequestType, TracerResponseType>
      factory_;
  grpc::CompletionQueue& cq_;
  TracerStubImpl stub_;
  AsyncStreamPtr<TracerRequestType, TracerResponseType> stream_;
  CircularBuffer<TracerRequestType> drained_messages_{DRAIN_BUFFER_SIZE};

  std::mutex mux_;
  std::condition_variable cv_;
};

class GrpcAsyncSegmentReporterStream final
    : public AsyncStream<TracerRequestType, TracerResponseType>,
      public AsyncStreamCallback {
 public:
  GrpcAsyncSegmentReporterStream(
      AsyncClient<TracerRequestType, TracerResponseType>& client,
      std::condition_variable& cv, const std::string& token);
  ~GrpcAsyncSegmentReporterStream() override;

  // AsyncStream
  void sendMessage(TracerRequestType message) override;

  // AsyncStreamCallback
  void onReady() override;
  void onIdle() override;
  void onWriteDone() override;
  void onReadDone() override {}

 private:
  bool clearPendingMessage();

  AsyncClient<TracerRequestType, TracerResponseType>& client_;
  TracerResponseType commands_;
  grpc::ClientContext ctx_;
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> request_writer_;
  CircularBuffer<TracerRequestType> pending_messages_{
      PENDING_MESSAGE_BUFFER_SIZE};
  StreamState state_{StreamState::Initialized};

  StreamCallbackTag ready_{StreamState::Ready, this};
  StreamCallbackTag write_done_{StreamState::WriteDone, this};

  std::condition_variable& cv_;
};

class GrpcAsyncSegmentReporterStreamBuilder final
    : public ClientStreamingStreamBuilder<TracerRequestType,
                                          TracerResponseType> {
 public:
  explicit GrpcAsyncSegmentReporterStreamBuilder(const std::string& token)
      : token_(token) {}

  // ClientStreamingStreamBuilder
  AsyncStreamPtr<TracerRequestType, TracerResponseType> create(
      AsyncClient<TracerRequestType, TracerResponseType>& client,
      std::condition_variable& cv) override;

 private:
  std::string token_;
};

}  // namespace cpp2sky
