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

#include "source/utils/exception.h"
#include "utils/grpc_status.h"

namespace cpp2sky {

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
    grpc::CompletionQueue* cq,
    AsyncStreamFactory<TracerRequestType, TracerResponseType>& factory,
    std::shared_ptr<grpc::ChannelCredentials> cred, std::string address)
    : address_(address), factory_(factory), cq_(cq) {
  stub_ = std::make_unique<TracerStubImpl>(grpc::CreateChannel(address, cred));
  stream_ = factory_.create(this);
  stream_->startStream();
}

void GrpcAsyncSegmentReporterClient::sendMessage(Message& message) {
  GPR_ASSERT(stream_ != nullptr);
  stream_->sendMessage(message);
}

std::unique_ptr<grpc::ClientAsyncWriter<TracerRequestType>>
GrpcAsyncSegmentReporterClient::createWriter(grpc::ClientContext* ctx,
                                             TracerResponseType* response,
                                             void* tag) {
  return stub_->createWriter(ctx, response, cq_, tag);
}

GrpcAsyncSegmentReporterStream::GrpcAsyncSegmentReporterStream(
    AsyncClient<TracerRequestType, TracerResponseType>* client)
    : client_(client) {}

GrpcAsyncSegmentReporterStream::~GrpcAsyncSegmentReporterStream() {
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

void GrpcAsyncSegmentReporterStream::sendMessage(Message& message) {
  pending_messages_.emplace(message);
  clearPendingMessages();
}

bool GrpcAsyncSegmentReporterStream::clearPendingMessages() {
  if (state_ != Operation::Idle || pending_messages_.empty()) {
    return false;
  }
  auto message = pending_messages_.back();
  pending_messages_.pop();
  TracerRequestType obj;
  obj.CopyFrom(message.get());
  request_writer_->Write(obj, toTag(&write_done_));
  return true;
}

bool GrpcAsyncSegmentReporterStream::handleOperation(Operation incoming_op) {
  state_ = incoming_op;
  while (true) {
    if (state_ == Operation::Connected) {
      gpr_log(GPR_INFO, "Established connection: %s",
        client_->peerAddress().c_str());
      state_ = Operation::Idle;
    } else if (state_ == Operation::WriteDone) {
      state_ = Operation::Idle;
      gpr_log(GPR_INFO, "Write finished");
    } else if (state_ == Operation::Idle) {
      gpr_log(GPR_INFO, "Stream idleing");
      // Release pending messages which are inserted when stream is not ready
      // to write.
      clearPendingMessages();
      return true;
    } else if (state_ == Operation::Finished) {
      gpr_log(GPR_INFO, "Stream closed with http status: %d",
              grpcStatusToGenericHttpStatus(status_.error_code()));
      if (!status_.ok()) {
        gpr_log(GPR_ERROR, "%s", status_.error_message().c_str());
      }
      return false;
    } else {
      throw TracerException("Unknown stream operation");
    }
  }
}

AsyncStreamPtr GrpcAsyncSegmentReporterStreamFactory::create(
    AsyncClient<TracerRequestType, TracerResponseType>* client) {
  if (client == nullptr) {
    return nullptr;
  }
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client);
}

}  // namespace cpp2sky
