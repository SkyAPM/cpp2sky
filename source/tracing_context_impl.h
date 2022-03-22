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

#include <string_view>

#include "cpp2sky/config.pb.h"
#include "cpp2sky/propagation.h"
#include "cpp2sky/tracing_context.h"
#include "source/utils/random_generator.h"

namespace cpp2sky {

class TracingSpanImpl : public TracingSpan {
 public:
  TracingSpanImpl(int32_t span_id, TracingContext& parent_tracing_context);

  skywalking::v3::SpanObject createSpanObject() override;

  int32_t spanId() const override { return span_id_; }
  int32_t parentSpanId() const override { return parent_span_id_; }
  int64_t startTime() const override { return start_time_; }
  int64_t endTime() const override { return end_time_; }
  const std::string& peer() const override { return peer_; }
  skywalking::v3::SpanType spanType() const override { return type_; }
  skywalking::v3::SpanLayer spanLayer() const override { return layer_; }
  bool errorStatus() const override { return is_error_; }
  bool skipAnalysis() const override { return skip_analysis_; }
  int32_t componentId() const override { return component_id_; }
  const std::vector<std::pair<std::string, std::string>>& tags()
      const override {
    return tags_;
  }
  const std::vector<skywalking::v3::Log>& logs() const override {
    return logs_;
  }
  bool finished() const override { return finished_; }
  std::string operationName() const override { return operation_name_; }

  void setParentSpanId(int32_t span_id) override {
    assert(!finished_);
    parent_span_id_ = span_id;
  }
  void startSpan(std::string_view operation_name) override;
  void startSpan(std::string_view operation_name,
                 TimePoint<SystemTime> current_time) override;
  void startSpan(std::string_view operation_name,
                 TimePoint<SteadyTime> current_time) override;
  void endSpan() override;
  void endSpan(TimePoint<SystemTime> current_time) override;
  void endSpan(TimePoint<SteadyTime> current_time) override;
  void setPeer(const std::string& remote_address) override {
    assert(!finished_);
    peer_ = remote_address;
  }
  void setPeer(std::string&& remote_address) override {
    assert(!finished_);
    peer_ = std::move(remote_address);
  }
  void setSpanType(skywalking::v3::SpanType type) override { type_ = type; }
  void setSpanLayer(skywalking::v3::SpanLayer layer) override {
    layer_ = layer;
  }
  void setErrorStatus() override { is_error_ = true; }
  void setSkipAnalysis() override { skip_analysis_ = true; }
  void addTag(std::string_view key, std::string_view value) override {
    assert(!finished_);
    tags_.emplace_back(key, value);
  }
  void addLog(std::string_view key, std::string_view value) override;
  void addLog(std::string_view key, std::string_view value,
              TimePoint<SystemTime> current_time) override;
  void addLog(std::string_view key, std::string_view value,
              TimePoint<SteadyTime> current_time) override;
  void setComponentId(int32_t component_id) override;
  void setOperationName(std::string_view name) override;

 private:
  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  int32_t span_id_;
  int32_t parent_span_id_;
  int64_t start_time_ = 0;
  int64_t end_time_ = 0;
  std::string operation_name_;
  std::string peer_;
  skywalking::v3::SpanType type_;
  skywalking::v3::SpanLayer layer_;
  // ComponentId is predefined by SkyWalking OAP. The range of id is 9000~9999
  // on C++ language SDK. Based on
  // https://github.com/apache/skywalking/blob/master/docs/en/guides/Component-library-settings.md
  int32_t component_id_ = 9000;
  bool is_error_ = false;
  std::vector<std::pair<std::string, std::string>> tags_;
  std::vector<skywalking::v3::Log> logs_;
  bool skip_analysis_ = false;
  bool finished_ = false;

  TracingContext& parent_tracing_context_;
};

class TracingContextImpl : public TracingContext {
 public:
  // This constructor is called when there is no parent SpanContext.
  TracingContextImpl(const std::string& service_name,
                     const std::string& instance_name, RandomGenerator& random);
  TracingContextImpl(const std::string& service_name,
                     const std::string& instance_name,
                     SpanContextPtr parent_span_context,
                     RandomGenerator& random);
  TracingContextImpl(const std::string& service_name,
                     const std::string& instance_name,
                     SpanContextPtr parent_span_context,
                     SpanContextExtensionPtr parent_ext_span_context,
                     RandomGenerator& random);

  const std::string& traceId() const override { return trace_id_; }
  const std::string& traceSegmentId() const override {
    return trace_segment_id_;
  }
  const std::string& service() const override { return service_; }
  const std::string& serviceInstance() const override {
    return service_instance_;
  }
  const std::list<TracingSpanPtr>& spans() const override { return spans_; }
  SpanContextPtr parentSpanContext() const override {
    return parent_span_context_;
  }
  SpanContextExtensionPtr parentSpanContextExtension() const override {
    return parent_ext_span_context_;
  }

  TracingSpanPtr createExitSpan(TracingSpanPtr parent_span) override;

  TracingSpanPtr createEntrySpan() override;
  std::optional<std::string> createSW8HeaderValue(
      const std::string_view target_address) override;
  skywalking::v3::SegmentObject createSegmentObject() override;
  void setSkipAnalysis() override { should_skip_analysis_ = true; }
  bool skipAnalysis() override { return should_skip_analysis_; }
  bool readyToSend() override;
  std::string logMessage(std::string_view message) const override;

 private:
  std::string encodeSpan(TracingSpanPtr parent_span,
                         const std::string_view target_address);
  TracingSpanPtr createSpan();

  SpanContextPtr parent_span_context_;
  SpanContextExtensionPtr parent_ext_span_context_;

  std::list<TracingSpanPtr> spans_;

  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  std::string trace_id_;
  std::string trace_segment_id_;
  std::string service_;
  std::string service_instance_;

  bool should_skip_analysis_ = false;
};

class TracingContextFactory {
 public:
  TracingContextFactory(const TracerConfig& config);

  TracingContextPtr create();
  TracingContextPtr create(SpanContextPtr span_context);
  TracingContextPtr create(SpanContextPtr span_context,
                           SpanContextExtensionPtr ext_span_context);

 private:
  std::string service_name_;
  std::string instance_name_;
  RandomGeneratorImpl random_generator_;
};

}  // namespace cpp2sky
