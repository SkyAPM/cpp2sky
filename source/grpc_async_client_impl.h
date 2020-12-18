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
#include "language-agent/Tracing.grpc.pb.h"
#include "language-agent/Tracing.pb.h"
#include "source/utils/circular_buffer.h"

#define DRAIN_BUFFER_SIZE 1024
#define PENDING_MESSAGE_BUFFER_SIZE 1024

namespace cpp2sky {

using TracerRequestType = SegmentObject;
using TracerResponseType = Commands;

class TracerStubImpl final
    : public TracerStub<TracerRequestType, TracerResponseType> {
 public:
  TracerStubImpl(std::shared_ptr<grpc::Channel> channel);

  // TracerStub
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> createWriter(
      grpc::ClientContext* ctx, TracerResponseType* response,
      grpc::CompletionQueue* cq, void* tag) override;

 private:
  std::unique_ptr<TraceSegmentReportService::Stub> stub_;
};

class GrpcAsyncSegmentReporterStream;

class GrpcAsyncSegmentReporterClient final
    : public AsyncClient<TracerRequestType, TracerResponseType> {
 public:
  GrpcAsyncSegmentReporterClient(
      const ClientConfig& config, grpc::CompletionQueue* cq,
      AsyncStreamFactory<TracerRequestType, TracerResponseType>& factory,
      std::shared_ptr<grpc::ChannelCredentials> cred);
  ~GrpcAsyncSegmentReporterClient();

  // AsyncClient
  void sendMessage(TracerRequestType message) override;
  std::string peerAddress() override { return address_; }
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> createWriter(
      grpc::ClientContext* ctx, TracerResponseType* response,
      void* tag) override;
  void drainPendingMessage(TracerRequestType pending_message) override {
    drained_messages_.push(pending_message);
  }
  void resetStream() override {
    if (stream_) {
      gpr_log(GPR_INFO, "Stream %p had destroyed.", stream_.get());
      stream_.reset();
    }
  }
  void startStream() override;
  size_t numOfMessages() override { return drained_messages_.size(); }

 private:
  std::string token_;
  std::string address_;
  AsyncStreamFactory<TracerRequestType, TracerResponseType>& factory_;
  TracerStubPtr<TracerRequestType, TracerResponseType> stub_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::Channel> channel_;
  AsyncStreamPtr<TracerRequestType> stream_;
  CircularBuffer<TracerRequestType> drained_messages_{DRAIN_BUFFER_SIZE};

  std::mutex mux_;
  std::condition_variable cv_;
};

struct TaggedStream {
  Operation operation;
  GrpcAsyncSegmentReporterStream* stream;
};

void* toTag(TaggedStream* stream);
TaggedStream* deTag(void* stream);

class GrpcAsyncSegmentReporterStream final
    : public AsyncStream<TracerRequestType> {
 public:
  GrpcAsyncSegmentReporterStream(
      AsyncClient<TracerRequestType, TracerResponseType>* client,
      std::condition_variable& cv);
  ~GrpcAsyncSegmentReporterStream() override;

  // AsyncStream
  bool startStream() override;
  void sendMessage(TracerRequestType message) override;
  void handleOperation(Operation incoming_op) override;
  void undrainMessage(TracerRequestType message) override {
    pending_messages_.push(message);
  }

 private:
  bool clearPendingMessages();

  AsyncClient<TracerRequestType, TracerResponseType>* client_;
  TracerResponseType commands_;
  grpc::ClientContext ctx_;
  std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>> request_writer_;
  CircularBuffer<TracerRequestType> pending_messages_{
      PENDING_MESSAGE_BUFFER_SIZE};
  Operation state_{Operation::Initialized};

  TaggedStream connected_{Operation::Connected, this};
  TaggedStream write_done_{Operation::WriteDone, this};

  std::condition_variable& cv_;
};

class GrpcAsyncSegmentReporterStreamFactory final
    : public AsyncStreamFactory<TracerRequestType, TracerResponseType> {
 public:
  // AsyncStreamFactory
  AsyncStreamPtr<TracerRequestType> create(
      AsyncClient<TracerRequestType, TracerResponseType>* client,
      std::condition_variable& cv) override;
};

}  // namespace cpp2sky
