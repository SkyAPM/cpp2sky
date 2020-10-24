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

#include <list>
#include <memory>
#include <unordered_map>

#include "language-agent/Tracing.pb.h"
#include "source/config_impl.h"
#include "source/propagation.h"

namespace cpp2sky {

class SegmentContext;
using SegmentContextPtr = std::shared_ptr<SegmentContext>;

class CurrentSegmentSpan {
 public:
  CurrentSegmentSpan(SegmentContext* segment_context, int32_t span_id);

  SpanObject createSpanObject();

  int32_t spanId() const { return span_id_; }

  void setParentSpanId(int32_t span_id) { parent_span_id_ = span_id; }
  void setStartTime(int64_t start_time) { start_time_ = start_time; }
  void setEndTime(int64_t end_time) { end_time_ = end_time; }
  void setOperationName(std::string& operation_name) {
    operation_name_ = operation_name;
  }
  void setPeer(std::string& remote_address) { peer_ = remote_address; }
  void setSpanType(SpanType type) { type_ = type; }
  void setSpanLayer(SpanLayer layer) { layer_ = layer; }
  void errorOccured() { is_error_ = true; }
  void skipAnalysis() { skip_analysis_ = true; }
  void addTag(std::string& key, std::string& value) {
    tags_.emplace(key, value);
  }
  void addLog(int64_t time, std::string& key, std::string& value);

 private:
  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  int32_t span_id_;
  int32_t parent_span_id_;
  int64_t start_time_;
  int64_t end_time_;
  std::string operation_name_;
  std::string peer_;
  SpanType type_;
  SpanLayer layer_;
  // ComponentId is predefined by SkyWalking OAP. The range of id is 9000~10000
  // on C++ language SDK. Based on
  // https://github.com/apache/skywalking/blob/master/docs/en/guides/Component-library-settings.md
  int32_t component_id_ = 9000;
  bool is_error_ = false;
  std::unordered_map<std::string, std::string> tags_;
  std::vector<Log> logs_;
  bool skip_analysis_ = false;

  SegmentContextPtr parent_segment_context_;
};

using CurrentSegmentSpanPtr = std::shared_ptr<CurrentSegmentSpan>;

class SegmentContext {
 public:
  // This constructor is called when there is no parent SpanContext.
  SegmentContext(Config& config);
  SegmentContext(Config& config, SpanContextPtr parent_span_context);

  const std::string& traceId() const { return trace_id_; }
  const std::string& traceSegmentId() const { return trace_segment_id_; }
  const std::string& service() const { return service_; }
  const std::string& serviceInstance() const { return service_instance_; }
  const std::list<CurrentSegmentSpanPtr>& spans() const { return spans_; }
  const SpanContext& parentSpanContext() const {
    return *parent_span_context_.get();
  }

  CurrentSegmentSpanPtr createCurrentSegmentSpan(
      CurrentSegmentSpanPtr parent_span);

  SegmentObject createSegmentObject();

 private:
  SpanContextPtr parent_span_context_;
  std::list<CurrentSegmentSpanPtr> spans_;

  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  std::string trace_id_;
  std::string trace_segment_id_;
  std::string service_;
  std::string service_instance_;
};

}  // namespace cpp2sky
