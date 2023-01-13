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
#include <thread>

#include "absl/strings/string_view.h"
#include "cpp2sky/exception.h"
#include "spdlog/spdlog.h"

namespace cpp2sky {

namespace {
static constexpr absl::string_view authenticationKey = "authentication";
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
  stream_ready_tag_ = StreamCallbackTag([this](bool event_ok) {
    if (event_ok) {
      assert(stream_ != nullptr);
      stream_ready_ = true;
      sendMessageImpl();
    } else {
      // Re-create stream.
      startStream();
    }
    return true;
  });

  send_message_tag_ = StreamCallbackTag([this](bool event_ok) {
    if (event_ok) {
      // Success and try to send next message.
      sendMessageImpl();
    } else {
      // Re-create stream.
      startStream();
    }
    return true;
  });

  event_notify_tag_ = StreamCallbackTag([this](bool) {
    if (stream_ready_) {
      sendMessageImpl();
    }
    return true;
  });

  startStream();
}

void GrpcAsyncSegmentReporterClient::sendMessageImpl() {
  auto message = pending_messages_.pop();
  if (message.has_value()) {
    stream_->sendMessage(message.value());
  }
}

void GrpcAsyncSegmentReporterClient::sendMessage(TracerRequestType message) {
  pending_messages_.push(message);

  if (!stream_) {
    info(
        "[Reporter] No active stream, inserted message into pending message "
        "queue. "
        "pending message size: {}",
        pending_messages_.size());
    return;
  }

  grpc::Alarm alarm;
  // Post a idle event to the completion queue.
  alarm.Set(&cq_, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME),
            &event_notify_tag_);
}

void GrpcAsyncSegmentReporterClient::startStream() {
  stream_ready_ = false;

  resetStream();
  stream_ = factory_->create(*this);
  info("[Reporter] Stream {} had created.", fmt::ptr(stream_.get()));
}

void GrpcAsyncSegmentReporterClient::resetStream() {
  if (stream_) {
    info("[Reporter] Stream {} has destroyed.", fmt::ptr(stream_.get()));
    stream_.reset();
  }
}

GrpcAsyncSegmentReporterStream::GrpcAsyncSegmentReporterStream(
    AsyncClient<TracerRequestType, TracerResponseType>& client,
    const std::string& token)
    : client_(dynamic_cast<GrpcAsyncSegmentReporterClient&>(client)) {
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
  request_writer_->StartCall(&client_.streamReadyTag());
}

void GrpcAsyncSegmentReporterStream::sendMessage(TracerRequestType message) {
  request_writer_->Write(message, &client_.sendMessageTag());
}

AsyncStreamSharedPtr<TracerRequestType, TracerResponseType>
GrpcAsyncSegmentReporterStreamBuilder::create(
    AsyncClient<TracerRequestType, TracerResponseType>& client) {
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client, token_);
}

}  // namespace cpp2sky
