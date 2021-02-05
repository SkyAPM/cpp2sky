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

namespace cpp2sky {

ConfigurationDiscoveryServiceStubImpl::ConfigurationDiscoveryServiceStubImpl(
    std::shared_ptr<grpc::Channel> channel)
    : stub_(skywalking::v3::ConfigurationDiscoveryService::NewStub(channel)) {}

std::unique_ptr<grpc::ClientAsyncResponseReader<CdsResponse>>
ConfigurationDiscoveryServiceStubImpl::createReader(grpc::ClientContext* ctx,
                                                    CdsRequest* request,
                                                    grpc::CompletionQueue* cq) {
  return stub_->PrepareAsyncfetchConfigurations(ctx, *request, cq);
}

GrpcAsyncConfigurationDiscoveryServiceClient::
    GrpcAsyncConfigurationDiscoveryServiceClient(
        const std::string& address, grpc::CompletionQueue* cq,
        AsyncStreamFactory<CdsRequest, CdsResponse>& factory,
        std::shared_ptr<grpc::ChannelCredentials> cred)
    : factory_(factory),
      cq_(cq),
      channel_(grpc::CreateChannel(address, cred)) {
  stub_ = std::make_unique<ConfigurationDiscoveryServiceStubImpl>(channel_);
}



}  // namespace cpp2sky
