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

#include "source/segment_context_impl.h"

#include <algorithm>

#include "language-agent/Tracing.pb.h"
#include "source/utils/base64.h"
#include "source/utils/random_generator.h"

namespace cpp2sky {

CurrentSegmentSpanImpl::CurrentSegmentSpanImpl(
    int32_t span_id, SegmentContext* parent_segment_context)
    : span_id_(span_id), parent_segment_context_(parent_segment_context) {}

SpanObject CurrentSegmentSpanImpl::createSpanObject() {
  SpanObject obj;

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

  auto* entry = obj.mutable_refs()->Add();
  entry->set_traceid(parent_segment_context_->traceId());
  entry->set_parenttracesegmentid(parent_segment_context_->traceSegmentId());
  entry->set_parentservice(parent_segment_context_->service());
  entry->set_parentserviceinstance(parent_segment_context_->serviceInstance());

  if (parent_segment_context_->parentSpanContext() != nullptr) {
    entry->set_parentspanid(
        parent_segment_context_->parentSpanContext()->spanId());
    entry->set_parentendpoint(
        parent_segment_context_->parentSpanContext()->endpoint());
  }

  for (auto& tag : tags_) {
    auto* entry = obj.mutable_tags()->Add();
    entry->set_key(tag.first);
    entry->set_value(tag.second);
  }

  for (auto& log : logs_) {
    auto* entry = obj.mutable_logs()->Add();
    *entry = log;
  }

  if (parent_segment_context_->parentSpanContextExtension() != nullptr) {
    if (parent_segment_context_->parentSpanContextExtension()->tracingMode() ==
        TracingMode::Skip) {
      obj.set_skipanalysis(true);
    }
  }
  return obj;
}

void CurrentSegmentSpanImpl::addLog(int64_t time, std::string& key,
                                    std::string& value) {
  Log l;
  l.set_time(time);
  auto* entry = l.add_data();
  entry->set_key(key);
  entry->set_value(value);
  logs_.emplace_back(l);
}

SegmentContextImpl::SegmentContextImpl(Config& config, RandomGenerator& random)
    : trace_id_(random.uuid()),
      trace_segment_id_(random.uuid()),
      service_(config.serviceName()),
      service_instance_(config.instanceName()) {}

SegmentContextImpl::SegmentContextImpl(
    Config& config, SpanContextPtr parent_span_context,
    SpanContextExtensionPtr parent_ext_span_context, RandomGenerator& random)
    : parent_span_context_(std::move(parent_span_context)),
      parent_ext_span_context_(std::move(parent_ext_span_context)),
      trace_id_(parent_span_context_->traceId()),
      trace_segment_id_(random.uuid()),
      service_(config.serviceName()),
      service_instance_(config.instanceName()) {}

SegmentContextImpl::SegmentContextImpl(Config& config,
                                       SpanContextPtr parent_span_context,
                                       RandomGenerator& random)
    : parent_span_context_(std::move(parent_span_context)),
      trace_id_(parent_span_context_->traceId()),
      trace_segment_id_(random.uuid()),
      service_(config.serviceName()),
      service_instance_(config.instanceName()) {}

CurrentSegmentSpanPtr SegmentContextImpl::createCurrentSegmentSpan(
    CurrentSegmentSpanPtr parent_span) {
  auto current_span =
      std::make_shared<CurrentSegmentSpanImpl>(spans_.size(), this);
  if (parent_span != nullptr) {
    current_span->setParentSpanId(parent_span->spanId());
    current_span->setSpanType(SpanType::Exit);
  } else {
    current_span->setParentSpanId(-1);
    current_span->setSpanType(SpanType::Entry);
  }
  // It supports only HTTP request tracing.
  current_span->setSpanLayer(SpanLayer::Http);
  spans_.push_back(current_span);
  return current_span;
}

SegmentObject SegmentContextImpl::createSegmentObject() {
  SegmentObject obj;
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

}  // namespace cpp2sky
