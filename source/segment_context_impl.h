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

#include "cpp2sky/config.pb.h"
#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "source/utils/random_generator.h"

namespace cpp2sky {

class CurrentSegmentSpanImpl : public CurrentSegmentSpan {
 public:
  CurrentSegmentSpanImpl(int32_t span_id,
                         SegmentContext& parent_segment_context);

  SpanObject createSpanObject() override;

#pragma region Getters
  int32_t spanId() const override { return span_id_; }
  int32_t parentSpanId() const override { return parent_span_id_; }
  int64_t startTime() const override { return start_time_; }
  int64_t endTime() const override { return end_time_; }
  const std::string& peer() const override { return peer_; }
  SpanType spanType() const override { return type_; }
  SpanLayer spanLayer() const override { return layer_; }
  bool errorStatus() const override { return is_error_; }
  bool skipAnalysis() const override { return skip_analysis_; }
  int32_t componentId() const override { return component_id_; }
  const std::vector<std::pair<std::string, std::string>>& tags()
      const override {
    return tags_;
  }
  const std::vector<Log>& logs() const override { return logs_; }
  bool finished() const override { return finished_; }
  std::string operationName() const override { return operation_name_; }
#pragma endregion

#pragma region Setters
  void setParentSpanId(int32_t span_id) override {
    assert(!finished_);
    parent_span_id_ = span_id;
  }
  void startSpan(std::string operation_name) override;
  void startSpan(std::string operation_name,
                 TimePoint<SystemTime> current_time) override;
  void startSpan(std::string operation_name,
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
  void setSpanType(SpanType type) override { type_ = type; }
  void setSpanLayer(SpanLayer layer) override { layer_ = layer; }
  void setErrorStatus() override { is_error_ = true; }
  void setSkipAnalysis() override { skip_analysis_ = true; }
  void addTag(std::string key, std::string value) override {
    assert(!finished_);
    tags_.emplace_back(key, value);
  }
  void addLog(std::string key, std::string value) override;
  void addLog(std::string key, std::string value,
              TimePoint<SystemTime> current_time) override;
  void addLog(std::string key, std::string value,
              TimePoint<SteadyTime> current_time) override;
  void setComponentId(int32_t component_id) override;
#pragma endregion

 private:
  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  int32_t span_id_;
  int32_t parent_span_id_;
  int64_t start_time_ = 0;
  int64_t end_time_ = 0;
  std::string operation_name_;
  std::string peer_;
  SpanType type_;
  SpanLayer layer_;
  // ComponentId is predefined by SkyWalking OAP. The range of id is 9000~9999
  // on C++ language SDK. Based on
  // https://github.com/apache/skywalking/blob/master/docs/en/guides/Component-library-settings.md
  int32_t component_id_ = 9000;
  bool is_error_ = false;
  std::vector<std::pair<std::string, std::string>> tags_;
  std::vector<Log> logs_;
  bool skip_analysis_ = false;
  bool finished_ = false;

  SegmentContext& parent_segment_context_;
};

class SegmentContextImpl : public SegmentContext {
 public:
  // This constructor is called when there is no parent SpanContext.
  SegmentContextImpl(const std::string& service_name,
                     const std::string& instance_name, RandomGenerator& random);
  SegmentContextImpl(const std::string& service_name,
                     const std::string& instance_name,
                     SpanContextPtr parent_span_context,
                     RandomGenerator& random);
  SegmentContextImpl(const std::string& service_name,
                     const std::string& instance_name,
                     SpanContextPtr parent_span_context,
                     SpanContextExtensionPtr parent_ext_span_context,
                     RandomGenerator& random);

#pragma region Getters
  const std::string& traceId() const override { return trace_id_; }
  const std::string& traceSegmentId() const override {
    return trace_segment_id_;
  }
  const std::string& service() const override { return service_; }
  const std::string& serviceInstance() const override {
    return service_instance_;
  }
  const std::list<CurrentSegmentSpanPtr>& spans() const override {
    return spans_;
  }
  SpanContextPtr parentSpanContext() const override {
    return parent_span_context_;
  }
  SpanContextExtensionPtr parentSpanContextExtension() const override {
    return parent_ext_span_context_;
  }
#pragma endregion

  CurrentSegmentSpanPtr createExitSpan(
      CurrentSegmentSpanPtr parent_span) override;

  CurrentSegmentSpanPtr createEntrySpan() override;
  std::optional<std::string> createSW8HeaderValue(
      const std::string_view target_address) override {
    return createSW8HeaderValue(nullptr, target_address);
  }
  std::optional<std::string> createSW8HeaderValue(
      CurrentSegmentSpanPtr parent_span,
      const std::string_view target_address) override;
  SegmentObject createSegmentObject() override;
  void setSkipAnalysis() override { should_skip_analysis_ = true; }
  bool skipAnalysis() override { return should_skip_analysis_; }
  bool readyToSend() override;

 private:
  std::string encodeSpan(CurrentSegmentSpanPtr parent_span,
                         const std::string_view target_address);
  CurrentSegmentSpanPtr createSpan();

  SpanContextPtr parent_span_context_;
  SpanContextExtensionPtr parent_ext_span_context_;

  std::list<CurrentSegmentSpanPtr> spans_;

  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  std::string trace_id_;
  std::string trace_segment_id_;
  std::string service_;
  std::string service_instance_;

  bool should_skip_analysis_ = false;
};

class SegmentContextFactoryImpl : public SegmentContextFactory {
 public:
  SegmentContextFactoryImpl(const TracerConfig& cfg);

  // SegmentContextFactory
  SegmentContextPtr create() override;
  SegmentContextPtr create(SpanContextPtr span_context) override;
  SegmentContextPtr create(SpanContextPtr span_context,
                           SpanContextExtensionPtr ext_span_context) override;

 private:
  std::string service_name_;
  std::string instance_name_;
  RandomGeneratorImpl random_generator_;
};

SegmentContextFactoryPtr createSegmentContextFactory(const TracerConfig& cfg) {
  return std::make_unique<SegmentContextFactoryImpl>(cfg);
}

}  // namespace cpp2sky
