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

#include <gmock/gmock.h>
#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "external/skywalking_data_collect_protocol/language-agent/Tracing.pb.h"
#include "mocks.h"
#include "source/propagation_impl.h"
#include "source/segment_context_impl.h"

using google::protobuf::util::JsonStringToMessage;
using testing::NiceMock;
using testing::Return;

namespace cpp2sky {

static constexpr std::string_view sample_ctx =
    "1-MQ==-NQ==-3-bWVzaA==-aW5zdGFuY2U=-L2FwaS92MS9oZWFsdGg=-"
    "ZXhhbXBsZS5jb206ODA4MA==";

class SegmentContextTest : public testing::Test {
 public:
  SegmentContextTest() {
    config_.set_service_name(service_name_);
    config_.set_instance_name(instance_name_);

    span_ctx_ = std::make_shared<SpanContextImpl>(sample_ctx);
    span_ext_ctx_ = std::make_shared<SpanContextExtensionImpl>("1");
  }

 protected:
  NiceMock<MockRandomGenerator> random_;
  std::string service_name_ = "mesh";
  std::string instance_name_ = "service_0";
  TracerConfig config_;
  SpanContextPtr span_ctx_;
  SpanContextExtensionPtr span_ext_ctx_;
};

TEST_F(SegmentContextTest, BasicTest) {
  SegmentContextImpl sc(config_.service_name(), config_.instance_name(),
                        random_);
  EXPECT_EQ(sc.service(), "mesh");
  EXPECT_EQ(sc.serviceInstance(), "service_0");

  // No parent span
  auto span = sc.createCurrentSegmentRootSpan();
  EXPECT_EQ(sc.spans().size(), 1);
  EXPECT_EQ(span->spanId(), 0);
  span->startSpan(false);
  span->setPeer("localhost:9000");
  span->endSpan(false);

  std::string json = R"EOF(
  {
    "spanId": "0",
    "parentSpanId": "-1",
    "startTime": "0",
    "endTime": "0",
    "peer": "localhost:9000",
    "spanType": "Entry",
    "spanLayer": "Http",
    "componentId": "9000"
  }
  )EOF";
  SpanObject expected_obj;
  JsonStringToMessage(json, &expected_obj);
  EXPECT_EQ(expected_obj.DebugString(), span->createSpanObject().DebugString());

  // With parent span
  auto span_child = sc.createCurrentSegmentSpan(std::move(span));
  EXPECT_EQ(sc.spans().size(), 2);
  EXPECT_EQ(span_child->spanId(), 1);
  span_child->startSpan(false);
  span_child->setPeer("localhost:9000");
  span_child->endSpan(false);

  std::string json2 = R"EOF(
  {
    "spanId": "1",
    "parentSpanId": "0",
    "startTime": "0",
    "endTime": "0",
    "peer": "localhost:9000",
    "spanType": "Exit",
    "spanLayer": "Http",
    "componentId": "9000"
  }
  )EOF";
  SpanObject expected_obj2;
  JsonStringToMessage(json2, &expected_obj2);
  EXPECT_EQ(expected_obj2.DebugString(),
            span_child->createSpanObject().DebugString());
}

TEST_F(SegmentContextTest, ChildSegmentContext) {
  SegmentContextImpl sc(config_.service_name(), config_.instance_name(),
                        span_ctx_, span_ext_ctx_, random_);
  EXPECT_EQ(sc.service(), "mesh");
  EXPECT_EQ(sc.serviceInstance(), "service_0");

  // No parent span
  auto span = sc.createCurrentSegmentRootSpan();
  EXPECT_EQ(sc.spans().size(), 1);
  EXPECT_EQ(span->spanId(), 0);
  span->startSpan(false);
  span->setPeer("localhost:9000");
  span->endSpan(false);

  std::string json = R"EOF(
  {
    "spanId": "0",
    "parentSpanId": "-1",
    "startTime": "0",
    "endTime": "0",
    "refs": {
      "refType": "CrossProcess",
      "traceId": "1",
      "parentTraceSegmentId": "5",
      "parentSpanId": 3,
      "parentService": "mesh",
      "parentServiceInstance": "instance",
      "parentEndpoint": "/api/v1/health",
      "networkAddressUsedAtPeer": "example.com:8080"
    },
    "peer": "localhost:9000",
    "spanType": "Entry",
    "spanLayer": "Http",
    "componentId": "9000",
    "skipAnalysis": "true"
  }
  )EOF";
  SpanObject expected_obj;
  JsonStringToMessage(json, &expected_obj);
  EXPECT_EQ(expected_obj.DebugString(), span->createSpanObject().DebugString());

  // With parent span
  auto span_child = sc.createCurrentSegmentSpan(std::move(span));
  EXPECT_EQ(sc.spans().size(), 2);
  EXPECT_EQ(span_child->spanId(), 1);
  span_child->startSpan(false);
  span_child->setPeer("localhost:9000");
  span_child->addTag("category", "database");
  std::string log_key = "service_0";
  std::string log_value = "error";
  span_child->addLog(log_key, log_value, false);
  span_child->endSpan(false);

  std::string json2 = R"EOF(
  {
    "spanId": "1",
    "parentSpanId": "0",
    "startTime": "0",
    "endTime": "0",
    "refs": {
      "refType": "CrossProcess",
      "traceId": "1",
      "parentTraceSegmentId": "5",
      "parentSpanId": 3,
      "parentService": "mesh",
      "parentServiceInstance": "instance",
      "parentEndpoint": "/api/v1/health",
      "networkAddressUsedAtPeer": "example.com:8080"
    },
    "peer": "localhost:9000",
    "spanType": "Exit",
    "spanLayer": "Http",
    "componentId": "9000",
    "skipAnalysis": "true",
    "tags": {
      "key": "category",
      "value": "database"
    },
    "logs": {
      "data": {
        "key": "service_0",
        "value": "error"
      }
    }
  }
  )EOF";
  SpanObject expected_obj2;
  JsonStringToMessage(json2, &expected_obj2);
  EXPECT_EQ(expected_obj2.DebugString(),
            span_child->createSpanObject().DebugString());
}

TEST_F(SegmentContextTest, SW8CreateTest) {
  SegmentContextImpl sc(config_.service_name(), config_.instance_name(),
                        span_ctx_, span_ext_ctx_, random_);
  EXPECT_EQ(sc.service(), "mesh");
  EXPECT_EQ(sc.serviceInstance(), "service_0");

  auto span = sc.createCurrentSegmentRootSpan();
  EXPECT_EQ(sc.spans().size(), 1);
  EXPECT_EQ(span->spanId(), 0);
  span->startSpan(false);
  span->setOperationName("/ping");
  span->endSpan(false);

  std::string target_address("10.0.0.1:443");
  std::string expect_sw8(
      "1-MQ==-dXVpZA==-0-bWVzaA==-c2VydmljZV8w-L3Bpbmc=-MTAuMC4wLjE6NDQz");

  EXPECT_EQ(expect_sw8, sc.createSW8HeaderValue(span, target_address));
}

}  // namespace cpp2sky
