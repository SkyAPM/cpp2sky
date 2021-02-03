
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
#include "cpp2sky/well_known_names.h"
#include "httplib.h"

using namespace cpp2sky;

TracerConfig config;

void init() {
  config.set_instance_name("node_0");
  config.set_service_name("consumer");
  config.set_address("collector:19876");
}

int main() {
  init();

  httplib::Server svr;
  auto tracer = createInsecureGrpcTracer(config);

  SegmentContextFactoryPtr factory = createSegmentContextFactory(config);

  svr.Get("/pong", [&](const httplib::Request& req, httplib::Response& res) {
    auto parent = req.get_header_value(kPropagationHeader.data());
    auto segment_context = factory->create(createSpanContext(parent));

    {
      StartEntrySpan entry_span(segment_context, "/pong");
    }

    tracer->sendSegment(std::move(segment_context));
  });

  svr.listen("0.0.0.0", 8080);
  return 0;
}
