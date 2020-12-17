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
                         SegmentContext* parent_segment_context);

  SpanObject createSpanObject() override;

#pragma region Getters
  int32_t spanId() const override { return span_id_; }
  std::string operationName() const override { return operation_name_; }
#pragma endregion

#pragma region Setters
  void setParentSpanId(int32_t span_id) override { parent_span_id_ = span_id; }
  void setStartTime(int64_t start_time) override { start_time_ = start_time; }
  void setEndTime(int64_t end_time) override { end_time_ = end_time; }
  void setOperationName(std::string& operation_name) override {
    operation_name_ = operation_name;
  }
  void setOperationName(std::string&& operation_name) override {
    operation_name_ = std::move(operation_name);
  }
  void setPeer(std::string& remote_address) override { peer_ = remote_address; }
  void setPeer(std::string&& remote_address) override {
    peer_ = std::move(remote_address);
  }
  void setSpanType(SpanType type) override { type_ = type; }
  void setSpanLayer(SpanLayer layer) override { layer_ = layer; }
  void errorOccured() override { is_error_ = true; }
  void skipAnalysis() override { skip_analysis_ = true; }
  void addTag(std::string& key, std::string& value) override {
    tags_.emplace(key, value);
  }
  void addTag(std::string&& key, std::string&& value) override {
    tags_.emplace(std::move(key), std::move(value));
  }
  void addLog(int64_t time, std::string& key, std::string& value) override;
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

  SegmentContext* parent_segment_context_;
};

class SegmentContextImpl : public SegmentContext {
 public:
  // This constructor is called when there is no parent SpanContext.
  SegmentContextImpl(SegmentConfig& config, RandomGenerator& random);
  SegmentContextImpl(SegmentConfig& config, SpanContextPtr parent_span_context,
                     RandomGenerator& random);
  SegmentContextImpl(SegmentConfig& config, SpanContextPtr parent_span_context,
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

  CurrentSegmentSpanPtr createCurrentSegmentSpan(
      CurrentSegmentSpanPtr parent_span) override;

  CurrentSegmentSpanPtr createCurrentSegmentRootSpan() override;
  std::string createSW8HeaderValue(CurrentSegmentSpanPtr parent_span,
                                   std::string& target_address,
                                   bool sample) override;
  std::string createSW8HeaderValue(CurrentSegmentSpanPtr parent,
                                   std::string&& target_address,
                                   bool sample = true) override;
  SegmentObject createSegmentObject() override;

 private:
  SpanContextPtr parent_span_context_;
  SpanContextExtensionPtr parent_ext_span_context_;

  std::list<CurrentSegmentSpanPtr> spans_;

  // Based on
  // https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto
  std::string trace_id_;
  std::string trace_segment_id_;
  std::string service_;
  std::string service_instance_;
};

SegmentContextPtr createSegmentContext(SegmentConfig& config,
                                       SpanContextPtr span_ctx,
                                       SpanContextExtensionPtr span_ctx_ext) {
  auto random_generator = RandomGeneratorImpl();
  return std::make_unique<SegmentContextImpl>(config, span_ctx, span_ctx_ext,
                                              random_generator);
}

SegmentContextPtr createSegmentContext(SegmentConfig& config,
                                       SpanContextPtr span_ctx) {
  auto random_generator = RandomGeneratorImpl();
  return std::make_unique<SegmentContextImpl>(config, span_ctx,
                                              random_generator);
}

SegmentContextPtr createSegmentContext(SegmentConfig& config) {
  auto random_generator = RandomGeneratorImpl();
  return std::make_unique<SegmentContextImpl>(config, random_generator);
}

}  // namespace cpp2sky
