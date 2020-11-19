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

void* toTag(TaggedStream* stream) { return reinterpret_cast<void*>(stream); }
TaggedStream* deTag(void* stream) { return static_cast<TaggedStream*>(stream); }

GrpcAsyncSegmentReporterClient::GrpcAsyncSegmentReporterClient(
    grpc::CompletionQueue* cq, AsyncStreamFactory<StubType>& factory,
    std::shared_ptr<grpc::ChannelCredentials> cred, std::string address)
    : cq_(cq),
      factory_(factory),
      stub_(TraceSegmentReportService::NewStub(
          grpc::CreateChannel(address, cred))) {
  stream_ = factory_.create(this);
  stream_->startStream();
}

GrpcAsyncSegmentReporterClient::~GrpcAsyncSegmentReporterClient() {
  if (stream_ != nullptr) {
    stream_->writeDone();
  }
}

bool GrpcAsyncSegmentReporterClient::sendMessage(Message& message) {
  GPR_ASSERT(stream_ != nullptr);
  return stream_->sendMessage(message);
}

GrpcAsyncSegmentReporterStream::GrpcAsyncSegmentReporterStream(
    AsyncClient<StubType>* client)
    : client_(client) {}

bool GrpcAsyncSegmentReporterStream::startStream() {
  request_writer_.reset();
  request_writer_ = client_->grpcStub()->Asynccollect(
      client_->grpcClientContext(), &commands_, client_->completionQueue(),
      toTag(&init_));
  return true;
}

bool GrpcAsyncSegmentReporterStream::sendMessage(Message& message) {
  if (!request_writer_) {
    return false;
  }
  SegmentObject obj;
  obj.CopyFrom(message);
  request_writer_->Write(obj, toTag(&write_));
  return true;
}

bool GrpcAsyncSegmentReporterStream::writeDone() {
  request_writer_->WritesDone(toTag(&write_done_));
  return true;
}

AsyncStreamPtr GrpcAsyncSegmentReporterStreamFactory::create(
    AsyncClient<StubType>* client) {
  if (client == nullptr) {
    return nullptr;
  }
  return std::make_shared<GrpcAsyncSegmentReporterStream>(client);
}

}  // namespace cpp2sky
