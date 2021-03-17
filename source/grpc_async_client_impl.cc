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

#include "grpc_async_client_impl.h"

#include <chrono>
#include <string_view>
#include <thread>

#include "cpp2sky/exception.h"
#include "spdlog/spdlog.h"

namespace cpp2sky {

namespace {
static constexpr std::string_view authenticationKey = "authentication";
}

using namespace spdlog;

GrpcAsyncSegmentReporterClient::GrpcAsyncSegmentReporterClient(
    const std::string& address, grpc::CompletionQueue& cq,
    ClientStreamingStreamBuilderPtr<TracerRequestType, TracerResponseType>
        factory,
    std::shared_ptr<grpc::ChannelCredentials> cred)
    : factory_(std::move(factory)),
      cq_(cq),
      stub_(grpc::CreateChannel(address, cred)) {
  startStream();
}

GrpcAsyncSegmentReporterClient::~GrpcAsyncSegmentReporterClient() {
  // It will wait until there is no drained messages.
  // There are no timeout option to handle this, so if you would like to stop
  // them, you should send signals like SIGTERM.
  // If server stopped with accidental issue, the event loop handle that it
  // failed to send message and close stream, then recreate new stream and try
  // to do it. This process will continue forever without sending explicit
  // signal.
  // TODO(shikugawa): Block to wait drained messages to be clear with createing
  // condition variable wrapper.
#ifndef GRPC_TEST
  if (stream_) {
    std::unique_lock<std::mutex> lck(mux_);
    while (!drained_messages_.empty()) {
      cv_.wait(lck);
    }
  }
#endif

  resetStream();
}

void GrpcAsyncSegmentReporterClient::sendMessage(TracerRequestType message) {
  if (!stream_) {
    drained_messages_.push(message);
    info(
        "[Reporter] No active stream, inserted message into draining message "
        "queue. "
        "pending message size: {}",
        drained_messages_.size());
    return;
  }
  stream_->sendMessage(message);
}

void GrpcAsyncSegmentReporterClient::startStream() {
  resetStream();

  stream_ = factory_->create(*this, cv_);
  info("[Reporter] Stream {} had created.", fmt::ptr(stream_.get()));

  const auto drained_messages_size = drained_messages_.size();

  while (!drained_messages_.empty()) {
    auto msg = drained_messages_.front();
    drained_messages_.pop();
    if (msg.has_value()) {
      stream_->sendMessage(msg.value());
    }
  }

  info("[Reporter] {} drained messages inserted into pending messages.",
       drained_messages_size);
}

void GrpcAsyncSegmentReporterClient::resetStream() {
  if (stream_) {
    info("[Reporter] Stream {} has destroyed.", fmt::ptr(stream_.get()));
    stream_.reset();
  }
}

GrpcAsyncSegmentReporterStream::GrpcAsyncSegmentReporterStream(
    AsyncClient<TracerRequestType, TracerResponseType>& client,
    std::condition_variable& cv, const std::string& token)
    : client_(client), cv_(cv) {
  if (!token.empty()) {
    ctx_.AddMetadata(authenticationKey.data(), token);
  }

  // Ensure pending RPC will complete if connection to the server is not
  // established first because of like server is not ready. This will queue
  // pending RPCs and when connection has established, Connected tag will be
  // sent to CompletionQueue.
  ctx_.set_wait_for_ready(true);

  request_writer_ = client_.stub().PrepareCall(
      &ctx_, "/TraceSegmentReportService/collect", &client_.completionQueue());
  request_writer_->StartCall(reinterpret_cast<void*>(&ready_));
}

GrpcAsyncSegmentReporterStream::~GrpcAsyncSegmentReporterStream() {
  const auto pending_messages_size = pending_messages_.size();
  while (!pending_messages_.empty()) {
    auto msg = pending_messages_.front();
    pending_messages_.pop();
    if (msg.has_value()) {
      client_.drainPendingMessage(msg.value());
    }
  }
  info("[Reporter] {} pending messages drained.", pending_messages_size);
}

void GrpcAsyncSegmentReporterStream::sendMessage(TracerRequestType message) {
  pending_messages_.push(message);
  clearPendingMessage();
}

bool GrpcAsyncSegmentReporterStream::clearPendingMessage() {
  if (state_ != StreamState::Idle || pending_messages_.empty()) {
    return false;
  }
  auto message = pending_messages_.front();
  if (!message.has_value()) {
    return false;
  }

  request_writer_->Write(message.value(),
                         reinterpret_cast<void*>(&write_done_));
  return true;
}

void GrpcAsyncSegmentReporterStream::onReady() {
  info("[Reporter] Stream ready");

  state_ = StreamState::Idle;
  onIdle();
}

void GrpcAsyncSegmentReporterStream::onIdle() {
  info("[Reporter] Stream idleing");

  // Release pending messages which are inserted when stream is not ready
  // to write.
  clearPendingMessage();

  if (pending_messages_.empty()) {
    cv_.notify_all();
  }
}

void GrpcAsyncSegmentReporterStream::onWriteDone() {
  info("[Reporter] Write finished");

  // Enqueue message after sending message finished.
  // With this, messages which failed to sent never lost even if connection
  // was closed. because pending messages with messages which failed to send
  // will drained and resend another stream.
  pending_messages_.pop();
  state_ = StreamState::Idle;

  onIdle();
}

AsyncStreamPtr<TracerRequestType, TracerResponseType>
GrpcAsyncSegmentReporterStreamBuilder::create(
    AsyncClient<TracerRequestType, TracerResponseType>& client,
    std::condition_variable& cv) {
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client, cv, token_);
}

}  // namespace cpp2sky
