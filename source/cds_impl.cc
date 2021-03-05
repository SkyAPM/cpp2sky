// Copyright 2021 SkyAPM

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cds_impl.h"

#include <condition_variable>

#include "spdlog/spdlog.h"

namespace cpp2sky {

using namespace spdlog;

GrpcAsyncConfigDiscoveryServiceClient::GrpcAsyncConfigDiscoveryServiceClient(
    const std::string& address, grpc::CompletionQueue& cq,
    UnaryStreamBuilderPtr<CdsRequest, CdsResponse> builder,
    std::shared_ptr<grpc::ChannelCredentials> cred)
    : builder_(std::move(builder)),
      cq_(cq),
      stub_(grpc::CreateChannel(address, cred)) {}

GrpcAsyncConfigDiscoveryServiceClient::
    ~GrpcAsyncConfigDiscoveryServiceClient() {
  resetStream();
}

void GrpcAsyncConfigDiscoveryServiceClient::sendMessage(CdsRequest request) {
  resetStream();
  stream_ = builder_->create(*this, request);
  info("[CDS] Stream {} had created", fmt::ptr(stream_.get()));
}

void GrpcAsyncConfigDiscoveryServiceClient::resetStream() {
  if (stream_) {
    info("[CDS] Stream {} has destroyed", fmt::ptr(this));
    stream_.reset();
  }
}

GrpcAsyncConfigDiscoveryServiceStream::GrpcAsyncConfigDiscoveryServiceStream(
    AsyncClient<CdsRequest, CdsResponse>& parent, CdsRequest request,
    DynamicConfig& config)
    : client_(parent), config_(config) {
  sendMessage(request);
}

void GrpcAsyncConfigDiscoveryServiceStream::sendMessage(CdsRequest request) {
  response_reader_ = client_.stub().PrepareUnaryCall(
      &ctx_, "/skywalking.v3.ConfigurationDiscoveryService/fetchConfigurations",
      request, &client_.completionQueue());
  response_reader_->StartCall();
  response_reader_->Finish(&commands_, &status_,
                           reinterpret_cast<void*>(&read_done_));
}

void GrpcAsyncConfigDiscoveryServiceStream::onReadDone() {
  info("[CDS] Stream {} read finished with gRPC status {}", fmt::ptr(this),
       static_cast<int>(status_.error_code()));

  if (status_.ok()) {
    config_.onConfigChange(commands_);
  }

  // Stream which finished to read done won't be destroyed here.
  // But it will be destroyed when new stream created.
}

AsyncStreamPtr<CdsRequest, CdsResponse>
GrpcAsyncConfigDiscoveryServiceStreamBuilder::create(
    AsyncClient<CdsRequest, CdsResponse>& client, CdsRequest request) {
  return std::make_shared<GrpcAsyncConfigDiscoveryServiceStream>(
      client, request, config_);
}

}  // namespace cpp2sky
