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

SegmentConfig seg_config;

void init() {
  seg_config.set_instance_name("node_0");
  seg_config.set_service_name("provider");
}

void requestPong(Tracer* tracer, SegmentContext* scp,
                 CurrentSegmentSpanPtr parent_span) {
  std::string target_address = "consumer:8080";
  auto current_span = scp->createCurrentSegmentSpan(parent_span);
  current_span->setStartTime(10100);
  current_span->setPeer(target_address);
  current_span->setOperationName("/pong");

  httplib::Client cli("consumer", 8080);
  httplib::Headers headers = {
      {"sw8", scp->createSW8HeaderValue(current_span, target_address)}};
  auto res = cli.Get("/pong", headers);

  current_span->setEndTime(10200);
}

void requestUsers(Tracer* tracer, SegmentContext* scp,
                  CurrentSegmentSpanPtr parent_span) {
  std::string target_address = "interm:8082";
  auto current_span = scp->createCurrentSegmentSpan(parent_span);
  current_span->setStartTime(10100);
  current_span->setPeer(target_address);
  current_span->setOperationName("/users");

  httplib::Client cli("interm", 8082);
  httplib::Headers headers = {
      {"sw8", scp->createSW8HeaderValue(current_span, target_address)}};
  auto res = cli.Get("/users", headers);

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

void handlePing2(Tracer* tracer, SegmentContext* scp, const httplib::Request&,
                 httplib::Response& response) {
  auto current_span = scp->createCurrentSegmentRootSpan();
  current_span->setStartTime(10000);
  current_span->setOperationName("/ping2");
  requestUsers(tracer, scp, current_span);
  current_span->setEndTime(20000);
}

int main() {
  init();

  TracerConfig tracer_config;
  auto* client_config = tracer_config.mutable_client_config();
  client_config->set_address("collector:19876");

  httplib::Server svr;
  auto tracer = createInsecureGrpcTracer(tracer_config);

  SegmentContextFactoryPtr factory = createSegmentContextFactory(seg_config);

  svr.Get("/ping", [&](const httplib::Request& req, httplib::Response& res) {
    auto current_segment = factory->create();
    handlePing(tracer.get(), current_segment.get(), req, res);
    tracer->sendSegment(std::move(current_segment));
  });

  svr.Get("/ping2", [&](const httplib::Request& req, httplib::Response& res) {
    auto current_segment = factory->create();
    handlePing2(tracer.get(), current_segment.get(), req, res);
    tracer->sendSegment(std::move(current_segment));
  });

  svr.listen("0.0.0.0", 8081);
  return 0;
}
