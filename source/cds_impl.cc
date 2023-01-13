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
  stream_ = builder_->create(*this);
  info("[CDS] Stream {} had created", fmt::ptr(stream_.get()));
  stream_->sendMessage(request);
}

void GrpcAsyncConfigDiscoveryServiceClient::resetStream() {
  if (stream_) {
    info("[CDS] Stream {} has destroyed", fmt::ptr(this));
    stream_.reset();
  }
}

GrpcAsyncConfigDiscoveryServiceStream::GrpcAsyncConfigDiscoveryServiceStream(
    AsyncClient<CdsRequest, CdsResponse>& parent, DynamicConfig& config)
    : client_(parent), config_(config) {
  read_tag_ = StreamCallbackTag([this](bool) {
    info("[CDS] Stream {} read finished with gRPC status {}", fmt::ptr(this),
         static_cast<int>(status_.error_code()));

    if (status_.ok()) {
      config_.onConfigChange(commands_);
    }
    return true;
  });
}

void GrpcAsyncConfigDiscoveryServiceStream::sendMessage(CdsRequest request) {
  response_reader_ = client_.stub().PrepareUnaryCall(
      &ctx_, "/skywalking.v3.ConfigurationDiscoveryService/fetchConfigurations",
      request, &client_.completionQueue());
  response_reader_->StartCall();
  response_reader_->Finish(&commands_, &status_, &read_tag_);
}

AsyncStreamSharedPtr<CdsRequest, CdsResponse>
GrpcAsyncConfigDiscoveryServiceStreamBuilder::create(
    AsyncClient<CdsRequest, CdsResponse>& client) {
  return std::make_shared<GrpcAsyncConfigDiscoveryServiceStream>(client,
                                                                 config_);
}

}  // namespace cpp2sky
