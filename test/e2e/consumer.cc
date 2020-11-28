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

static const std::string service_name = "consumer";
static const std::string instance_name = "node_0";
static const std::string address = "collector:19876";

Config config(service_name, instance_name, "");

void handlePong(Tracer* tracer, SegmentContext* scp,
                const httplib::Request& req, httplib::Response& response) {
  auto span = scp->createCurrentSegmentRootSpan();
  span->setStartTime(10110);
  span->setOperationName("/pong");
  span->setEndTime(10190);
}

int main() {
  httplib::Server svr;
  auto tracer = createInsecureGrpcTracer(address);

  svr.Get("/pong", [&](const httplib::Request& req, httplib::Response& res) {
    if (req.has_header("sw8")) {
      auto parent = req.get_header_value("sw8");
      auto parent_span = createSpanContext(parent);
      auto current_segment = createSegmentContext(config, parent_span);
      handlePong(tracer.get(), current_segment.get(), req, res);
      tracer->sendSegment(current_segment->createSegmentObject());
    }
  });

  svr.listen("0.0.0.0", 8080);
  return 0;
}
