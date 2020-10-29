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
#include <string>
#include <string_view>

namespace cpp2sky {

class SpanContext {
 public:
  SpanContext(std::string_view header_value);

  bool mustSend() const { return must_send_; }
  const std::string& traceId() const { return trace_id_; }
  const std::string& traceSegmentId() const { return trace_segment_id_; }
  int32_t spanId() const { return span_id_; }
  const std::string& service() const { return service_; }
  const std::string& serviceInstance() const { return service_instance_; }
  const std::string& endpoint() const { return endpoint_; }
  const std::string& targetAddress() const { return target_address_; }

 private:
  // Based on
  // https://github.com/apache/skywalking/blob/master/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v3.md
  bool must_send_ = false;
  std::string trace_id_;
  std::string trace_segment_id_;
  int32_t span_id_;
  std::string service_;
  std::string service_instance_;
  std::string endpoint_;
  std::string target_address_;
};

using SpanContextPtr = std::shared_ptr<SpanContext>;

enum TracingMode {
  Default,
  // It represents all spans generated in this context should skip
  // analysis.
  Skip
};

class SpanContextExtension {
 public:
  SpanContextExtension(std::string_view header_value);

  TracingMode tracingMode() const { return tracing_mode_; }

 private:
  TracingMode tracing_mode_ = TracingMode::Default;
};

using SpanContextExtensionPtr = std::shared_ptr<SpanContextExtension>;

}  // namespace cpp2sky