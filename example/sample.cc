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

#include <chrono>
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
  config.set_service_name("");
  config.set_address("0.0.0.0:11800");
}

int main() {
  init();

  httplib::Server svr;
  // 1. Create tracer object to send span data to OAP.
  auto tracer = createInsecureGrpcTracer(config);

  svr.Get("/ping", [&](const httplib::Request& req, httplib::Response& res) {
    std::string context = req.get_header_value(kPropagationHeader.data());

    SegmentContextPtr segment_context;
    if (!context.empty()) {
      // 2. Create segment context with propagated information.
      segment_context = tracer->newSegment(createSpanContext(context));
    }

    {
      // 3. Create entry span.
      StartEntrySpan current_span(segment_context, "sample_op3");

      /**
       * something....
       */
    }

    // 4. Send span data
    if (segment_context != nullptr) {
      tracer->sendSegment(std::move(segment_context));
    }
  });

  svr.listen("0.0.0.0", 8081);
  return 0;
}
