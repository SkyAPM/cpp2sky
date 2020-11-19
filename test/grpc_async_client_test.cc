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
#include <gtest/gtest.h>

#include <memory>

#include "language-agent/Tracing.pb.h"
#include "source/grpc_async_client_impl.h"
#include "test/mocks.h"

namespace cpp2sky {

using testing::_;

class GrpcAsyncSegmentReporterClientTest : public testing::Test {
 public:
  GrpcAsyncSegmentReporterClientTest() {
    EXPECT_CALL(factory_, create(_));
    EXPECT_CALL(*stream_, startStream());
    client_ = std::make_unique<GrpcAsyncSegmentReporterClient>(
        &cq_, factory_, grpc::InsecureChannelCredentials(), address_);
  }

 protected:
  grpc::CompletionQueue cq_;
  std::string address_{"localhost:50051"};
  std::shared_ptr<MockAsyncStream> stream_{std::make_shared<MockAsyncStream>()};
  MockAsyncStreamFactory<StubType> factory_{stream_};
  std::unique_ptr<GrpcAsyncSegmentReporterClient> client_;
};

TEST_F(GrpcAsyncSegmentReporterClientTest, SendMessageTest) {
  SegmentObject fake_message;
  EXPECT_CALL(*stream_, sendMessage(_)).WillOnce(Return(true));
  EXPECT_CALL(*stream_, writeDone());
  EXPECT_TRUE(client_->sendMessage(fake_message));
}

class GrpcAsyncSegmentReporterStreamTest : public testing::Test {
 public:
  GrpcAsyncSegmentReporterStreamTest() {
    ON_CALL(client_, grpcStub()).WillByDefault(Return(stub_.get()));
  }

 protected:
  std::string address_{"localhost:50051"};
  std::unique_ptr<StubType> stub_{TraceSegmentReportService::NewStub(
      grpc::CreateChannel(address_, grpc::InsecureChannelCredentials()))};
  MockAsyncClient<StubType> client_;
  GrpcAsyncSegmentReporterStream stream_{&client_};
};

TEST_F(GrpcAsyncSegmentReporterStreamTest, SendMessageTest) {
  EXPECT_CALL(client_, grpcStub).Times(2);
  EXPECT_CALL(client_, completionQueue).Times(2);
  EXPECT_CALL(client_, grpcClientContext).Times(2);
  EXPECT_TRUE(stream_.startStream());
}

}  // namespace cpp2sky
