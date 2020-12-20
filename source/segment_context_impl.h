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

#include <unordered_map>

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
  std::string operationName() const override { return operation_name_; }
#pragma endregion

#pragma region Setters
  void setParentSpanId(int32_t span_id) override { parent_span_id_ = span_id; }
  void setStartTime(int64_t start_time) override { start_time_ = start_time; }
  void setEndTime(int64_t end_time) override { end_time_ = end_time; }
  void setOperationName(const std::string& operation_name) override {
    operation_name_ = operation_name;
  }
  void setOperationName(std::string&& operation_name) override {
    operation_name_ = std::move(operation_name);
  }
  void setPeer(const std::string& remote_address) override {
    peer_ = remote_address;
  }
  void setPeer(std::string&& remote_address) override {
    peer_ = std::move(remote_address);
  }
  void setSpanType(SpanType type) override { type_ = type; }
  void setSpanLayer(SpanLayer layer) override { layer_ = layer; }
  void errorOccured() override { is_error_ = true; }
  void skipAnalysis() override { skip_analysis_ = true; }
  void addTag(const std::string& key, const std::string& value) override {
    tags_.emplace(key, value);
  }
  void addTag(std::string&& key, std::string&& value) override {
    tags_.emplace(std::move(key), std::move(value));
  }
  void addLog(int64_t time, const std::string& key,
              const std::string& value) override;
  void setComponentId(int32_t component_id) override;
#pragma endregion

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
  // ComponentId is predefined by SkyWalking OAP. The range of id is 9000~9999
  // on C++ language SDK. Based on
  // https://github.com/apache/skywalking/blob/master/docs/en/guides/Component-library-settings.md
  int32_t component_id_ = 9000;
  bool is_error_ = false;
  std::unordered_map<std::string, std::string> tags_;
  std::vector<Log> logs_;
  bool skip_analysis_ = false;

  SegmentContext& parent_segment_context_;
};

class SegmentContextImpl : public SegmentContext {
 public:
  // This constructor is called when there is no parent SpanContext.
  SegmentContextImpl(const SegmentConfig& config, RandomGenerator& random);
  SegmentContextImpl(const SegmentConfig& config,
                     SpanContextPtr parent_span_context,
                     RandomGenerator& random);
  SegmentContextImpl(const SegmentConfig& config,
                     SpanContextPtr parent_span_context,
                     SpanContextExtensionPtr parent_ext_span_context,
                     RandomGenerator& random);

#pragma region Setters
  void disableSampling() override;
#pragma endregion

#pragma region Getters
  bool samplingStatus() const override { return sample_; }
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
  bool sample_ = true;
};

class SegmentContextFactoryImpl : public SegmentContextFactory {
 public:
  SegmentContextFactoryImpl(const SegmentConfig& config);

  // SegmentContextFactory
  SegmentContextPtr create() override;
  SegmentContextPtr create(SpanContextPtr span_context) override;
  SegmentContextPtr create(SpanContextPtr span_context,
                           SpanContextExtensionPtr ext_span_context) override;

 private:
  const SegmentConfig config_;
  RandomGeneratorImpl random_generator_;
};

SegmentContextFactoryPtr createSegmentContextFactory(const SegmentConfig& cfg) {
  return std::make_unique<SegmentContextFactoryImpl>(cfg);
}

}  // namespace cpp2sky
