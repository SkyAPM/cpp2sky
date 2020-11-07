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
    config_ =
        std::make_unique<Config>(service_name_, instance_name_, token_);
    span_ctx_ = std::make_shared<SpanContextImpl>(sample_ctx);
    span_ext_ctx_ = std::make_shared<SpanContextExtensionImpl>("1");
  }

 protected:
  NiceMock<MockRandomGenerator> random_;
  std::string service_name_ = "mesh";
  std::string instance_name_ = "service_0";
  std::string token_ = "dummy";
  std::unique_ptr<Config> config_;
  SpanContextPtr span_ctx_;
  SpanContextExtensionPtr span_ext_ctx_;
};

TEST_F(SegmentContextTest, BasicTest) {
  SegmentContextImpl sc(*config_.get(), random_);
  EXPECT_EQ(sc.service(), "mesh");
  EXPECT_EQ(sc.serviceInstance(), "service_0");

  // No parent span
  auto span = sc.createCurrentSegmentSpan(nullptr);
  EXPECT_EQ(sc.spans().size(), 1);
  EXPECT_EQ(span->spanId(), 0);
  span->setStartTime(10000);
  span->setEndTime(20000);
  span->setPeer("localhost:9000");

  std::string json = R"EOF(
  {
    "spanId": "0",
    "parentSpanId": "-1",
    "startTime": "10000",
    "endTime": "20000",
    "refs": {
      "traceId": "uuid",
      "parentTraceSegmentId": "uuid",
      "parentService": "mesh",
      "parentServiceInstance": "service_0"
    },
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
  span_child->setStartTime(10000);
  span_child->setEndTime(12000);
  span_child->setPeer("localhost:9000");

  std::string json2 = R"EOF(
  {
    "spanId": "1",
    "parentSpanId": "0",
    "startTime": "10000",
    "endTime": "12000",
    "refs": {
      "traceId": "uuid",
      "parentTraceSegmentId": "uuid",
      "parentService": "mesh",
      "parentServiceInstance": "service_0"
    },
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
  SegmentContextImpl sc(*config_.get(), span_ctx_, span_ext_ctx_, random_);
  EXPECT_EQ(sc.service(), "mesh");
  EXPECT_EQ(sc.serviceInstance(), "service_0");

  // No parent span
  auto span = sc.createCurrentSegmentSpan(nullptr);
  EXPECT_EQ(sc.spans().size(), 1);
  EXPECT_EQ(span->spanId(), 0);
  span->setStartTime(10000);
  span->setEndTime(20000);
  span->setPeer("localhost:9000");

  std::string json = R"EOF(
  {
    "spanId": "0",
    "parentSpanId": "-1",
    "startTime": "10000",
    "endTime": "20000",
    "refs": {
      "traceId": "1",
      "parentTraceSegmentId": "uuid",
      "parentSpanId": 3,
      "parentService": "mesh",
      "parentServiceInstance": "service_0",
      "parentEndpoint": "/api/v1/health"
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
  span_child->setStartTime(10000);
  span_child->setEndTime(12000);
  span_child->setPeer("localhost:9000");
  span_child->addTag("category", "database");

  std::string log_key = "service_0";
  std::string log_value = "error";
  span_child->addLog(10500, log_key, log_value);

  std::string json2 = R"EOF(
  {
    "spanId": "1",
    "parentSpanId": "0",
    "startTime": "10000",
    "endTime": "12000",
    "refs": {
      "traceId": "1",
      "parentTraceSegmentId": "uuid",
      "parentSpanId": 3,
      "parentService": "mesh",
      "parentServiceInstance": "service_0",
      "parentEndpoint": "/api/v1/health"
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
      "time": "10500",
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

}  // namespace cpp2sky
