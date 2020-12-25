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

#include <map>

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
  bool samplingStatus() const override { return do_sample_; }
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
  void startSpan(bool set_time) override;
  void endSpan(bool set_time) override;
  void setOperationName(const std::string& operation_name) override {
    assert(!finished_);
    operation_name_ = operation_name;
  }
  void setOperationName(std::string&& operation_name) override {
    assert(!finished_);
    operation_name_ = std::move(operation_name);
  }
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
  void errorOccured() override { is_error_ = true; }
  void skipAnalysis() override { skip_analysis_ = true; }
  void addTag(const std::string& key, const std::string& value) override {
    assert(!finished_);
    tags_.emplace_back(key, value);
  }
  void addTag(std::string&& key, std::string&& value) override {
    assert(!finished_);
    tags_.emplace_back(std::move(key), std::move(value));
  }
  void addLog(const std::string& key, const std::string& value,
              bool set_time) override;
  void setComponentId(int32_t component_id) override;
  void setSamplingStatus(bool do_sample) override {
    assert(!finished_);
    do_sample_ = do_sample;
  }
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
  SegmentContext& parent_segment_context_;
  bool do_sample_ = true;
  bool finished_ = false;
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

#pragma region Setters
  void setDefaultSamplingStatus(bool do_sample) override;
#pragma endregion

#pragma region Getters
  bool defaultSamplingStatus() const override { return do_sample_default_; }
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

  CurrentSegmentSpanPtr createCurrentSegmentSpan(
      CurrentSegmentSpanPtr parent_span) override;

  CurrentSegmentSpanPtr createCurrentSegmentRootSpan() override;
  std::string createSW8HeaderValue(const std::string& target_address) override {
    return createSW8HeaderValue(nullptr, target_address);
  }
  std::string createSW8HeaderValue(std::string&& target_address) override {
    return createSW8HeaderValue(nullptr, target_address);
  }
  std::string createSW8HeaderValue(CurrentSegmentSpanPtr parent_span,
                                   const std::string& target_address) override;
  std::string createSW8HeaderValue(CurrentSegmentSpanPtr parent,
                                   std::string&& target_address) override;
  SegmentObject createSegmentObject() override;

  bool readyToSend() override;

 private:
  std::string encodeSpan(CurrentSegmentSpanPtr parent_span,
                         const std::string& target_address);

  SpanContextPtr parent_span_context_;
  SpanContextExtensionPtr parent_ext_span_context_;

  std::list<CurrentSegmentSpanPtr> spans_;

  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  std::string trace_id_;
  std::string trace_segment_id_;
  std::string service_;
  std::string service_instance_;

  bool is_root_ = false;
  // Sampling flag. It will send to OAP if propagated span context doesn't have
  // sampleing flag. When this context is root. It is configurable whether
  // sample or not.
  bool do_sample_default_ = true;
};

class SegmentContextFactoryImpl : public SegmentContextFactory {
 public:
  SegmentContextFactoryImpl(const TracerConfig& cfg);

  // SegmentContextFactory
  SegmentContextPtr create(bool default_sampling_status) override;
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
