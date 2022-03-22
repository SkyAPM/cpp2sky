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

#include "source/tracing_context_impl.h"

#include <string>
#include <string_view>

#include "cpp2sky/exception.h"
#include "cpp2sky/time.h"
#include "language-agent/Tracing.pb.h"
#include "source/utils/base64.h"
#include "source/utils/random_generator.h"

namespace cpp2sky {

TracingSpanImpl::TracingSpanImpl(int32_t span_id,
                                 TracingContext& parent_tracing_context)
    : span_id_(span_id), parent_tracing_context_(parent_tracing_context) {}

skywalking::v3::SpanObject TracingSpanImpl::createSpanObject() {
  skywalking::v3::SpanObject obj;

  obj.set_spanid(span_id_);
  obj.set_parentspanid(parent_span_id_);
  obj.set_starttime(start_time_);
  obj.set_endtime(end_time_);
  obj.set_operationname(operation_name_);
  obj.set_spantype(type_);
  obj.set_spanlayer(layer_);
  obj.set_componentid(component_id_);
  obj.set_iserror(is_error_);
  obj.set_peer(peer_);
  obj.set_skipanalysis(skip_analysis_);

  auto parent_span = parent_tracing_context_.parentSpanContext();
  // Inject request parent to the current segment.
  if (parent_span != nullptr) {
    auto* entry = obj.mutable_refs()->Add();
    // TODO(shikugawa): cpp2sky only supports cross process propagation right
    // now. So It is correct to specify this.
    entry->set_reftype(skywalking::v3::RefType::CrossProcess);
    entry->set_traceid(parent_span->traceId());
    entry->set_parenttracesegmentid(parent_span->traceSegmentId());
    entry->set_parentservice(parent_span->service());
    entry->set_parentserviceinstance(parent_span->serviceInstance());
    entry->set_parentspanid(parent_span->spanId());
    entry->set_parentendpoint(parent_span->endpoint());
    entry->set_networkaddressusedatpeer(parent_span->targetAddress());
  }

  for (auto& tag : tags_) {
    auto* entry = obj.mutable_tags()->Add();
    entry->set_key(std::string(tag.first));
    entry->set_value(std::string(tag.second));
  }

  for (auto& log : logs_) {
    auto* entry = obj.mutable_logs()->Add();
    *entry = log;
  }

  return obj;
}

void TracingSpanImpl::addLog(std::string_view key, std::string_view value) {
  assert(!finished_);
  auto now = TimePoint<SystemTime>();
  addLog(key, value, now);
}

void TracingSpanImpl::addLog(std::string_view key, std::string_view value,
                             TimePoint<SystemTime> current_time) {
  assert(!finished_);
  skywalking::v3::Log l;
  l.set_time(current_time.fetch());
  auto* entry = l.add_data();
  entry->set_key(std::string(key));
  entry->set_value(std::string(value));
  logs_.emplace_back(l);
}

void TracingSpanImpl::addLog(std::string_view key, std::string_view value,
                             TimePoint<SteadyTime> current_time) {
  assert(!finished_);
  skywalking::v3::Log l;
  l.set_time(current_time.fetch());
  auto* entry = l.add_data();
  entry->set_key(std::string(key));
  entry->set_value(std::string(value));
  logs_.emplace_back(l);
}

void TracingSpanImpl::startSpan(std::string_view operation_name) {
  auto now = TimePoint<SystemTime>();
  startSpan(operation_name, now);
}

void TracingSpanImpl::startSpan(std::string_view operation_name,
                                TimePoint<SystemTime> current_time) {
  operation_name_ = operation_name;
  start_time_ = current_time.fetch();
}

void TracingSpanImpl::startSpan(std::string_view operation_name,
                                TimePoint<SteadyTime> current_time) {
  operation_name_ = operation_name;
  start_time_ = current_time.fetch();
}

void TracingSpanImpl::endSpan() {
  assert(!finished_);
  auto now = TimePoint<SystemTime>();
  endSpan(now);
}

void TracingSpanImpl::endSpan(TimePoint<SystemTime> current_time) {
  end_time_ = current_time.fetch();
  finished_ = true;
}

void TracingSpanImpl::endSpan(TimePoint<SteadyTime> current_time) {
  end_time_ = current_time.fetch();
  finished_ = true;
}

void TracingSpanImpl::setComponentId(int32_t component_id) {
  assert(!finished_);

  // Component ID is reserved on Skywalking spec.
  // For more details here:
  // https://github.com/apache/skywalking/blob/master/docs/en/guides/Component-library-settings.md
  component_id_ = component_id;
}

void TracingSpanImpl::setOperationName(std::string_view name) {
  assert(!finished_);
  operation_name_ = name;
}

TracingContextImpl::TracingContextImpl(const std::string& service_name,
                                       const std::string& instance_name,
                                       RandomGenerator& random)
    : trace_id_(random.uuid()),
      trace_segment_id_(random.uuid()),
      service_(service_name),
      service_instance_(instance_name) {}

TracingContextImpl::TracingContextImpl(
    const std::string& service_name, const std::string& instance_name,
    SpanContextPtr parent_span_context,
    SpanContextExtensionPtr parent_ext_span_context, RandomGenerator& random)
    : parent_span_context_(std::move(parent_span_context)),
      parent_ext_span_context_(std::move(parent_ext_span_context)),
      trace_id_(parent_span_context_->traceId()),
      trace_segment_id_(random.uuid()),
      service_(service_name),
      service_instance_(instance_name) {}

TracingContextImpl::TracingContextImpl(const std::string& service_name,
                                       const std::string& instance_name,
                                       SpanContextPtr parent_span_context,
                                       RandomGenerator& random)
    : parent_span_context_(std::move(parent_span_context)),
      trace_id_(parent_span_context_->traceId()),
      trace_segment_id_(random.uuid()),
      service_(service_name),
      service_instance_(instance_name) {}

TracingSpanPtr TracingContextImpl::createExitSpan(TracingSpanPtr parent_span) {
  auto current_span = createSpan();
  current_span->setParentSpanId(parent_span->spanId());
  current_span->setSpanType(skywalking::v3::SpanType::Exit);
  return current_span;
}

TracingSpanPtr TracingContextImpl::createEntrySpan() {
  if (!spans_.empty()) {
    return nullptr;
  }

  auto current_span = createSpan();
  current_span->setParentSpanId(-1);
  current_span->setSpanType(skywalking::v3::SpanType::Entry);
  return current_span;
}

std::optional<std::string> TracingContextImpl::createSW8HeaderValue(
    const std::string_view target_address) {
  auto target_span = spans_.back();
  if (target_span->spanType() != skywalking::v3::SpanType::Exit) {
    return std::nullopt;
  }
  return encodeSpan(target_span, target_address);
}

std::string TracingContextImpl::encodeSpan(
    TracingSpanPtr parent_span, const std::string_view target_address) {
  assert(parent_span);
  std::string header_value;

  auto parent_spanid = std::to_string(parent_span->spanId());
  auto endpoint = spans_.front()->operationName();

  // always send to OAP
  header_value += "1-";
  header_value += Base64::encode(trace_id_) + "-";
  header_value += Base64::encode(trace_segment_id_) + "-";
  header_value += parent_spanid + "-";
  header_value += Base64::encode(service_) + "-";
  header_value += Base64::encode(service_instance_) + "-";
  header_value += Base64::encode(endpoint) + "-";
  header_value +=
      Base64::encode(target_address.data(), target_address.length());

  return header_value;
}

TracingSpanPtr TracingContextImpl::createSpan() {
  auto current_span = std::make_shared<TracingSpanImpl>(spans_.size(), *this);

  // It supports only HTTP request tracing.
  current_span->setSpanLayer(skywalking::v3::SpanLayer::Http);
  if (should_skip_analysis_) {
    current_span->setSkipAnalysis();
  }

  spans_.push_back(current_span);
  return current_span;
}

skywalking::v3::SegmentObject TracingContextImpl::createSegmentObject() {
  skywalking::v3::SegmentObject obj;

  obj.set_traceid(trace_id_);
  obj.set_tracesegmentid(trace_segment_id_);
  obj.set_service(service_);
  obj.set_serviceinstance(service_instance_);

  for (auto& span : spans_) {
    auto* entry = obj.mutable_spans()->Add();
    *entry = span->createSpanObject();
  }

  return obj;
}

bool TracingContextImpl::readyToSend() {
  for (const auto& span : spans_) {
    if (!span->finished()) {
      return false;
    }
  }
  return true;
}

std::string TracingContextImpl::logMessage(std::string_view message) const {
  std::string output = message.data();
  output += "\", \"SW_CTX\": [";
  output += "\"" + service_ + "\",";
  output += "\"" + service_instance_ + "\",";
  output += "\"" + trace_id_ + "\",";
  output += "\"" + trace_segment_id_ + "\",";

  if (!spans_.empty()) {
    output += "\"" + std::to_string(spans_.back()->spanId()) + "\"]}";
  } else {
    output += "\"-1\"]}";
  }

  return output;
}

TracingContextFactory::TracingContextFactory(const TracerConfig& config)
    : service_name_(config.service_name()),
      instance_name_(config.instance_name()) {}

TracingContextPtr TracingContextFactory::create() {
  return std::make_unique<TracingContextImpl>(service_name_, instance_name_,
                                              random_generator_);
}

TracingContextPtr TracingContextFactory::create(SpanContextPtr span_context) {
  return std::make_unique<TracingContextImpl>(service_name_, instance_name_,
                                              span_context, random_generator_);
}

TracingContextPtr TracingContextFactory::create(
    SpanContextPtr span_context, SpanContextExtensionPtr ext_span_context) {
  auto context = std::make_unique<TracingContextImpl>(
      service_name_, instance_name_, span_context, ext_span_context,
      random_generator_);
  if (ext_span_context->tracingMode() == TracingMode::Skip) {
    context->setSkipAnalysis();
  }
  return context;
}

}  // namespace cpp2sky
