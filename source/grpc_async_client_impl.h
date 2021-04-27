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

#include <grpcpp/generic/generic_stub.h>
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

namespace cpp2sky {

namespace {
static constexpr size_t pending_message_buffer_size = 1024;
}

using TracerRequestType = skywalking::v3::SegmentObject;
using TracerResponseType = skywalking::v3::Commands;

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
  CircularBuffer<TracerRequestType>& pendingMessages() override {
    return pending_messages_;
  }
  void startStream() override;
  grpc::TemplatedGenericStub<TracerRequestType, TracerResponseType>& stub()
      override {
    return stub_;
  }
  grpc::CompletionQueue& completionQueue() override { return cq_; }

  size_t numOfMessages() { return pending_messages_.size(); }

 private:
  void resetStream();

  std::string address_;
  ClientStreamingStreamBuilderPtr<TracerRequestType, TracerResponseType>
      factory_;
  grpc::CompletionQueue& cq_;
  grpc::TemplatedGenericStub<TracerRequestType, TracerResponseType> stub_;
  AsyncStreamPtr<TracerRequestType, TracerResponseType> stream_;
  CircularBuffer<TracerRequestType> pending_messages_{
      pending_message_buffer_size};

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

  // AsyncStream
  void sendMessage(TracerRequestType message) override;

  // AsyncStreamCallback
  void onReady() override;
  void onIdle() override;
  void onWriteDone() override;
  void onReadDone() override {}
  void onStreamFinish() override { client_.startStream(); }

 private:
  bool clearPendingMessage();

  AsyncClient<TracerRequestType, TracerResponseType>& client_;
  TracerResponseType commands_;
  grpc::ClientContext ctx_;
  std::unique_ptr<
      grpc::ClientAsyncReaderWriter<TracerRequestType, TracerResponseType>>
      request_writer_;
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
