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

#include <iostream>
#include <string>

#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "cpp2sky/tracer.h"

using namespace cpp2sky;

int main(void) {
  Config config("dummy", "instance_3", "dummy_token");
  std::string sw8 =
      "1-MQ==-NQ==-3-bWVzaA==-aW5zdGFuY2U=-L2FwaS92MS9oZWFsdGg=-"
      "ZXhhbXBsZS5jb206ODA4MA==";
  auto span_ctx = createSpanContext(sw8);
  auto current_segment = createSegmentContext(config, span_ctx);

  auto tracer = createInsecureGrpcTracer("localhost:11800");

  tracer->sendSegment(std::move(current_segment));
}
