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

#pragma once

#include <thread>

#include "cpp2sky/internal/async_client.h"
#include "cpp2sky/internal/matcher.h"
#include "cpp2sky/tracer.h"
#include "language-agent/ConfigurationDiscoveryService.pb.h"
#include "source/cds_impl.h"
#include "source/grpc_async_client_impl.h"
#include "source/tracing_context_impl.h"
#include "source/utils/timer.h"

namespace cpp2sky {

using TracerRequestType = skywalking::v3::SegmentObject;
using TracerResponseType = skywalking::v3::Commands;

using CdsRequest = skywalking::v3::ConfigurationSyncRequest;
using CdsResponse = skywalking::v3::Commands;

class TracerImpl : public Tracer {
 public:
  TracerImpl(TracerConfig& config,
             std::shared_ptr<grpc::ChannelCredentials> cred);
  TracerImpl(
      TracerConfig& config,
      AsyncClientPtr<TracerRequestType, TracerResponseType> reporter_client);
  ~TracerImpl();

  TracingContextPtr newContext() override;
  TracingContextPtr newContext(SpanContextPtr span) override;

  bool report(TracingContextPtr obj) override;

 private:
  void init(TracerConfig& config,
            std::shared_ptr<grpc::ChannelCredentials> cred);
  void run();
  void cdsRequest();

  std::unique_ptr<Timer> cds_timer_;
  DynamicConfig config_;
  AsyncClientPtr<TracerRequestType, TracerResponseType> reporter_client_;
  AsyncClientPtr<CdsRequest, CdsResponse> cds_client_;
  grpc::CompletionQueue cq_;
  std::thread evloop_thread_;
  TracingContextFactory segment_factory_;
  std::list<MatcherPtr> op_name_matchers_;
};

}  // namespace cpp2sky
