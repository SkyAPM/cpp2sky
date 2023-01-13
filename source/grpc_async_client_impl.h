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

#include <grpcpp/alarm.h>
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

  // AsyncClient
  void sendMessage(TracerRequestType message) override;
  void startStream() override;

  grpc::TemplatedGenericStub<TracerRequestType, TracerResponseType>& stub()
      override {
    return stub_;
  }
  grpc::CompletionQueue& completionQueue() override { return cq_; }

  StreamCallbackTag& streamReadyTag() { return stream_ready_tag_; }
  StreamCallbackTag& sendMessageTag() { return send_message_tag_; }
  StreamCallbackTag& eventNotifyTag() { return event_notify_tag_; }

 private:
  void sendMessageImpl();

  void resetStream();

  std::atomic<bool> stream_ready_{};

  std::string address_;
  ClientStreamingStreamBuilderPtr<TracerRequestType, TracerResponseType>
      factory_;

  StreamCallbackTag stream_ready_tag_;
  StreamCallbackTag send_message_tag_;
  StreamCallbackTag event_notify_tag_;

  grpc::CompletionQueue& cq_;
  grpc::TemplatedGenericStub<TracerRequestType, TracerResponseType> stub_;
  AsyncStreamSharedPtr<TracerRequestType, TracerResponseType> stream_;
  CircularBuffer<TracerRequestType> pending_messages_{
      pending_message_buffer_size};
};

class GrpcAsyncSegmentReporterStream final
    : public AsyncStream<TracerRequestType, TracerResponseType> {
 public:
  GrpcAsyncSegmentReporterStream(
      AsyncClient<TracerRequestType, TracerResponseType>& client,
      const std::string& token);

  // AsyncStream
  void sendMessage(TracerRequestType) override;

 private:
  GrpcAsyncSegmentReporterClient& client_;
  grpc::ClientContext ctx_;
  std::unique_ptr<
      grpc::ClientAsyncReaderWriter<TracerRequestType, TracerResponseType>>
      request_writer_;
};

class GrpcAsyncSegmentReporterStreamBuilder final
    : public ClientStreamingStreamBuilder<TracerRequestType,
                                          TracerResponseType> {
 public:
  explicit GrpcAsyncSegmentReporterStreamBuilder(const std::string& token)
      : token_(token) {}

  // ClientStreamingStreamBuilder
  AsyncStreamSharedPtr<TracerRequestType, TracerResponseType> create(
      AsyncClient<TracerRequestType, TracerResponseType>& client) override;

 private:
  std::string token_;
};

}  // namespace cpp2sky
