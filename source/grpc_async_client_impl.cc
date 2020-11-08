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

#include "utils/grpc_status.h"

namespace cpp2sky {

GrpcAsyncSegmentReporterClient::GrpcAsyncSegmentReporterClient(
    grpc::CompletionQueue& cq, std::shared_ptr<grpc::Channel> channel,
    AsyncStreamFactory<StubType>& factory)
    : cq_(cq), stub_(channel), factory_(factory) {}

void GrpcAsyncSegmentReporterClient::onSendMessage(const Message& message) {
  auto stream = factory_.create(this);
  streams_.emplace_back(stream);
  stream->setData(message);
  stream->startStream();
}

void GrpcAsyncSegmentReporterStream::setData(const Message& message) {
  SegmentObject obj;
  obj.CopyFrom(message);
  data_ = obj;
  data_set_ = true;
}

const Message& GrpcAsyncSegmentReporterStream::reply() const {
  return static_cast<const Message&>(commands_);
}

void GrpcAsyncSegmentReporterStream::startStream() {
  request_writer_ = parent_->grpcStub().PrepareAsynccollect(
      &parent_->grpcClientContext(), &commands_, &parent_->grpcDispatchQueue());
  if (request_writer_ == nullptr) {
    return;
  }
  request_writer_->StartCall(this);
  if (data_set_) {
    request_writer_->Write(data_, this);
  }
  request_writer_->WritesDone(this);
}

uint16_t GrpcAsyncSegmentReporterStream::status() const {
  return grpcStatusToGenericHttpStatus(status_.error_code());
}

AsyncStreamPtr GrpcAsyncSegmentReporterStreamFactory::create(
    AsyncClient<StubType>* client) {
  if (client == nullptr) {
    return nullptr;
  }
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client);
}

}  // namespace cpp2sky
