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
#include "utils/grpc_status.h"

#define DEFAULT_CONNECTION_ACTIVE_RETRY_TIMES 5
#define DEFAULT_CONNECTION_ACTIVE_RETRY_SLEEP_SEC 3

namespace cpp2sky {

namespace {
static constexpr std::string_view authenticationKey = "authentication";
}

void* toTag(TaggedStream* stream) { return reinterpret_cast<void*>(stream); }
TaggedStream* deTag(void* stream) { return static_cast<TaggedStream*>(stream); }

TracerStubImpl::TracerStubImpl(std::shared_ptr<grpc::Channel> channel)
    : stub_(TraceSegmentReportService::NewStub(channel)) {}

std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>>
TracerStubImpl::createWriter(grpc::ClientContext* ctx,
                             TracerResponseType* response,
                             grpc::CompletionQueue* cq, void* tag) {
  return stub_->Asynccollect(ctx, response, cq, tag);
}

GrpcAsyncSegmentReporterClient::GrpcAsyncSegmentReporterClient(
    const std::string& address, const std::string& token,
    grpc::CompletionQueue* cq,
    AsyncStreamFactory<TracerRequestType, TracerResponseType>& factory,
    std::shared_ptr<grpc::ChannelCredentials> cred)
    : token_(token),
      factory_(factory),
      cq_(cq),
      channel_(grpc::CreateChannel(address, cred)) {
  stub_ = std::make_unique<TracerStubImpl>(channel_);
  startStream();
}

GrpcAsyncSegmentReporterClient::~GrpcAsyncSegmentReporterClient() {
  // If connection is inactive, it dispose all drained messages even if it has
  // tons of messages.
  uint8_t retry_times = DEFAULT_CONNECTION_ACTIVE_RETRY_TIMES;
  while (channel_->GetState(false) !=
         grpc_connectivity_state::GRPC_CHANNEL_READY) {
    if (retry_times <= 0) {
      gpr_log(GPR_INFO,
              "All %ld pending messages have disposed because of no active "
              "connection",
              drained_messages_.size());
      resetStream();
      return;
    }
    retry_times--;
    std::this_thread::sleep_for(
        std::chrono::seconds(DEFAULT_CONNECTION_ACTIVE_RETRY_SLEEP_SEC));
  }

  // It will wait until there is no drained messages.
  // There are no timeout option to handle this, so if you would like to stop
  // them, you should send signals like SIGTERM.
  // If server stopped with accidental issue, the event loop handle that it
  // failed to send message and close stream, then recreate new stream and try
  // to do it. This process will continue forever without sending explicit
  // signal.
  {
    std::unique_lock<std::mutex> lck(mux_);
    while (!drained_messages_.empty()) {
      cv_.wait(lck);
    }
  }

  resetStream();
}

void GrpcAsyncSegmentReporterClient::sendMessage(TracerRequestType message) {
  if (!stream_) {
    drained_messages_.push(message);
    gpr_log(GPR_INFO,
            "No active stream, inserted message into draining message queue. "
            "pending message size: %ld",
            drained_messages_.size());
    return;
  }
  stream_->sendMessage(message);
}

std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>>
GrpcAsyncSegmentReporterClient::createWriter(grpc::ClientContext* ctx,
                                             TracerResponseType* response,
                                             void* tag) {
  if (!token_.empty()) {
    ctx->AddMetadata(authenticationKey.data(), token_);
  }
  return stub_->createWriter(ctx, response, cq_, tag);
}

void GrpcAsyncSegmentReporterClient::startStream() {
  resetStream();

  stream_ = factory_.create(this, cv_);

  const auto drained_messages_size = drained_messages_.size();
  while (!drained_messages_.empty()) {
    auto msg = drained_messages_.front();
    drained_messages_.pop();
    if (msg.has_value()) {
      stream_->undrainMessage(msg.value());
    }
  }
  gpr_log(GPR_INFO, "%ld drained messages inserted into pending messages.",
          drained_messages_size);

  stream_->startStream();
  gpr_log(GPR_INFO, "Stream %p had created.", stream_.get());
}

GrpcAsyncSegmentReporterStream::GrpcAsyncSegmentReporterStream(
    AsyncClient<TracerRequestType, TracerResponseType>* client,
    std::condition_variable& cv)
    : client_(client), cv_(cv) {}

GrpcAsyncSegmentReporterStream::~GrpcAsyncSegmentReporterStream() {
  const auto pending_messages_size = pending_messages_.size();
  while (!pending_messages_.empty()) {
    auto msg = pending_messages_.front();
    pending_messages_.pop();
    if (msg.has_value()) {
      client_->drainPendingMessage(msg.value());
    }
  }
  gpr_log(GPR_INFO, "%ld pending messages drained.", pending_messages_size);

  ctx_.TryCancel();
  request_writer_->Finish(&status_, toTag(&finish_));
}

bool GrpcAsyncSegmentReporterStream::startStream() {
  request_writer_.reset();

  // Ensure pending RPC will complete if connection to the server is not
  // established first because of like server is not ready. This will queue
  // pending RPCs and when connection has established, Connected tag will be
  // sent to CompletionQueue.
  ctx_.set_wait_for_ready(true);
  request_writer_ =
      client_->createWriter(&ctx_, &commands_, toTag(&connected_));
  return true;
}

void GrpcAsyncSegmentReporterStream::sendMessage(TracerRequestType message) {
  pending_messages_.push(message);
  clearPendingMessages();
}

bool GrpcAsyncSegmentReporterStream::clearPendingMessages() {
  if (state_ != Operation::Idle || pending_messages_.empty()) {
    return false;
  }
  auto message = pending_messages_.front();
  if (!message.has_value()) {
    return false;
  }
  request_writer_->Write(message.value(), toTag(&write_done_));
  return true;
}

bool GrpcAsyncSegmentReporterStream::handleOperation(Operation incoming_op) {
  state_ = incoming_op;
  if (state_ == Operation::Connected) {
    gpr_log(GPR_INFO, "Established connection: %s",
            client_->peerAddress().c_str());
    state_ = Operation::Idle;
  } else if (state_ == Operation::WriteDone) {
    gpr_log(GPR_INFO, "Write finished");
    // Enqueue message after sending message finished.
    // With this, messages which failed to sent never lost even if connection
    // was closed. because pending messages with messages which failed to send
    // will drained and resend another stream.
    pending_messages_.pop();
    state_ = Operation::Idle;
  }

  if (state_ == Operation::Idle) {
    gpr_log(GPR_INFO, "Stream idleing");
    // Release pending messages which are inserted when stream is not ready
    // to write.
    clearPendingMessages();

    if (pending_messages_.empty()) {
      cv_.notify_all();
    }
    return true;
  } else if (state_ == Operation::Finished) {
    gpr_log(GPR_INFO, "Stream closed with http status: %d",
            grpcStatusToGenericHttpStatus(status_.error_code()));
    if (!status_.ok()) {
      gpr_log(GPR_ERROR, "%s", status_.error_message().c_str());
    }
    return false;
  }
  throw TracerException("Unknown stream operation");
}

AsyncStreamPtr<TracerRequestType> GrpcAsyncSegmentReporterStreamFactory::create(
    AsyncClient<TracerRequestType, TracerResponseType>* client,
    std::condition_variable& cv) {
  if (client == nullptr) {
    return nullptr;
  }
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client, cv);
}

}  // namespace cpp2sky
