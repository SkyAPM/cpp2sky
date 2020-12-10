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
#include "httplib.h"

using namespace cpp2sky;

static const std::string service_name = "";
static const std::string instance_name = "client_0";
static const std::string address = "0.0.0.0:11800";

SegmentConfig seg_config(service_name, instance_name);
TracerConfig tracer_config(address);

uint64_t now() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

int main() {
  // 1. Create tracer object to send span data to OAP.
  auto tracer = createInsecureGrpcTracer(tracer_config);
  // 2. Create segment context
  auto current_segment = createSegmentContext(seg_config);

  // 3. Initialize span data to track root workload on current service.
  auto current_span = current_segment->createCurrentSegmentRootSpan();

  // 4. Set info
  current_span->setOperationName("/ping");
  current_span->setStartTime(now());

  httplib::Client cli("remote", 8082);
  httplib::Headers headers = {{"sw8", current_segment->createSW8HeaderValue(
                                          current_span, "remote:8082")}};
  auto res = cli.Get("/ping", headers);

  current_span->setEndTime(now());

  // 5. Send span data
  tracer->sendSegment(std::move(current_segment));
  std::cout << "finished" << std::endl;
  return 0;
}
