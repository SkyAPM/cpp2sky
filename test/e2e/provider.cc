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

#include <string>

#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "cpp2sky/tracer.h"
#include "httplib.h"

using namespace cpp2sky;

static const std::string service_name = "provider";
static const std::string instance_name = "node_0";
static const std::string address = "collector:19876";

Config config(service_name, instance_name, "");

void requestPong(Tracer* tracer, SegmentContext* scp,
                 CurrentSegmentSpanPtr parent_span) {
  std::string target_address = "consumer:8080";
  auto current_span = scp->createCurrentSegmentSpan(parent_span);
  current_span->setStartTime(10100);
  current_span->setOperationName("/pong");

  httplib::Client cli("consumer", 8080);
  httplib::Headers headers = {
      {"sw8", scp->createSW8HeaderValue(current_span, target_address)}};
  auto res = cli.Get("/pong", headers);

  current_span->setEndTime(10200);
}

void handlePing(Tracer* tracer, SegmentContext* scp, const httplib::Request&,
                httplib::Response& response) {
  auto current_span = scp->createCurrentSegmentRootSpan();
  current_span->setStartTime(10000);
  current_span->setOperationName("/ping");
  requestPong(tracer, scp, current_span);
  current_span->setEndTime(20000);
}

int main() {
  httplib::Server svr;
  auto tracer = createInsecureGrpcTracer(address);

  svr.Get("/ping", [&](const httplib::Request& req, httplib::Response& res) {
    auto current_segment = createSegmentContext(config);
    handlePing(tracer.get(), current_segment.get(), req, res);
    tracer->sendSegment(current_segment->createSegmentObject());
  });

  svr.listen("0.0.0.0", 8081);
  return 0;
}
