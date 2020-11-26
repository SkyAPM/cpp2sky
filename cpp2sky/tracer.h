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

#include <memory>

#include "language-agent/Tracing.pb.h"

namespace cpp2sky {

class Tracer {
 public:
  virtual ~Tracer() = default;

  /**
   * Send SegmentObject to the collector. (lvalue)
   */
  virtual void sendSegment(SegmentObject& obj) = 0;

  /**
   * Send SegmentObject to the collector. (rvalue)
   */
  virtual void sendSegment(SegmentObject&& obj) = 0;
};

using TracerPtr = std::unique_ptr<Tracer>;

TracerPtr createInsecureGrpcTracer(std::string address);

}  // namespace cpp2sky
