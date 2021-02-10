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

namespace cpp2sky {

ConfigDiscoveryServiceStubImpl::ConfigDiscoveryServiceStubImpl(
    std::shared_ptr<grpc::Channel> channel)
    : stub_(skywalking::v3::ConfigurationDiscoveryService::NewStub(channel)) {}

std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>>
ConfigDiscoveryServiceStubImpl::createReader(grpc::ClientContext* ctx,
                                             CdsRequest* request,
                                             grpc::CompletionQueue* cq) {
  return stub_->PrepareAsyncfetchConfigurations(ctx, *request, cq);
}

GrpcAsyncConfigDiscoveryServiceClient:: GrpcAsyncConfigDiscoveryServiceClient(
      const std::string& address, grpc::CompletionQueue& cq,
      AsyncStreamFactoryPtr<CdsRequest, CdsResponse> factory,
      std::shared_ptr<grpc::ChannelCredentials> cred
    : factory_(factory), cq_(cq), channel_(grpc::CreateChannel(address, cred)) {
  stub_ = std::make_unique<ConfigDiscoveryServiceStubImpl>(channel_);
}

void GrpcAsyncConfigDiscoveryServiceClient::sendMessage(CdsRequest request) {
  resetStream();

  std::condition_variable cv;
  stream_ = factory_->create(*this, cv);

  gpr_log(GPR_INFO, "[CDS] Stream %p had created.", stream_.get());
}

GrpcAsyncConfigDiscoveryServiceStream::GrpcAsyncConfigDiscoveryServiceStream(
    GrpcAsyncConfigDiscoveryServiceClient& parent, CdsRequest request)
    : client_(parent) {
  sendMessage(request);
}

void GrpcAsyncConfigDiscoveryServiceStream::sendMessage(CdsRequest request) {
  response_reader_ = parent.stub().createReader(&ctx_, &request, &client_.completionQueue());
  response_reader_->StartCall();
  response_reader_->Finish(&commands_, &status_, reinterpret_cast<void*>(read_done_));
}

void GrpcAsyncConfigDiscoveryServiceStream::onReadDone() {}

AsyncStreamPtr<CdsRequest, CdsResponse> GrpcAsyncConfigDiscoveryServiceStreamFactory::create(
    AsyncClient<TracerRequestType, TracerResponseType>& client,
    std::condition_variable& cv) {
  return std::make_shared<GrpcAsyncConfigDiscoveryServiceStream>(client);
}

}  // namespace cpp2sky
