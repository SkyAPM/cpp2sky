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
  MockAsyncStreamFactory<TracerRequestType, TracerResponseType> factory_{
      stream_};
  std::unique_ptr<GrpcAsyncSegmentReporterClient> client_;
};

TEST_F(GrpcAsyncSegmentReporterClientTest, SendMessageTest) {
  SegmentObject fake_message;
  EXPECT_CALL(*stream_, sendMessage(_));
  client_->sendMessage(fake_message);
}

}  // namespace cpp2sky
