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
    EXPECT_CALL(factory_, create(_, _));
    EXPECT_CALL(*stream_, startStream());

    config_.set_address(address_);
    config_.set_token(token_);

    client_ = std::make_unique<GrpcAsyncSegmentReporterClient>(
        config_.address(), config_.token(), &cq_, factory_,
        grpc::InsecureChannelCredentials());
  }

 protected:
  TracerConfig config_;
  grpc::CompletionQueue cq_;
  std::string address_{"localhost:50051"};
  std::string token_{"token"};
  std::shared_ptr<MockAsyncStream<TracerRequestType>> stream_{
      std::make_shared<MockAsyncStream<TracerRequestType>>()};
  MockAsyncStreamFactory<TracerRequestType, TracerResponseType> factory_{
      stream_};
  std::unique_ptr<GrpcAsyncSegmentReporterClient> client_;
};

TEST_F(GrpcAsyncSegmentReporterClientTest, SendMessageTest) {
  SegmentObject fake_message;
  EXPECT_CALL(*stream_, sendMessage(_));
  client_->sendMessage(fake_message);
}

TEST_F(GrpcAsyncSegmentReporterClientTest, MessageDrainTest) {
  std::queue<TracerRequestType> fake_pending_messages;
  for (int i = 0; i < 3; ++i) {
    fake_pending_messages.emplace(SegmentObject());
  }
  while (fake_pending_messages.size() != 0) {
    auto msg = fake_pending_messages.front();
    fake_pending_messages.pop();
    client_->drainPendingMessage(msg);
  }
  EXPECT_EQ(fake_pending_messages.size(), 0);
  EXPECT_EQ(client_->numOfMessages(), 3);
}

}  // namespace cpp2sky
