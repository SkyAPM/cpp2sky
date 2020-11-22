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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cpp2sky/internal/async_client.h"
#include "cpp2sky/internal/random_generator.h"

using testing::_;
using testing::Return;

namespace cpp2sky {

class MockRandomGenerator : public RandomGenerator {
 public:
  MockRandomGenerator() { ON_CALL(*this, uuid).WillByDefault(Return("uuid")); }
  MOCK_METHOD(std::string, uuid, ());
};

class MockAsyncStream : public AsyncStream {
 public:
  MOCK_METHOD(bool, startStream, ());
  MOCK_METHOD(void, sendMessage, (Message&));
  MOCK_METHOD(std::string, peerAddress, ());
  MOCK_METHOD(bool, handleOperation, (Operation));
};

template <class StubType>
class MockAsyncClient : public AsyncClient<StubType> {
 public:
  MockAsyncClient() {
    ON_CALL(*this, completionQueue()).WillByDefault(Return(&cq_));
  }

  MOCK_METHOD(void, sendMessage, (Message&));
  MOCK_METHOD(grpc::CompletionQueue*, completionQueue, ());
  MOCK_METHOD(StubType*, grpcStub, ());
  MOCK_METHOD(std::string, peerAddress, ());

 private:
  grpc::CompletionQueue cq_;
};

template <class StubType>
class MockAsyncStreamFactory : public AsyncStreamFactory<StubType> {
 public:
  MockAsyncStreamFactory(AsyncStreamPtr stream) : stream_(stream) {
    ON_CALL(*this, create(_)).WillByDefault(Return(stream_));
  }
  MOCK_METHOD(AsyncStreamPtr, create, (AsyncClient<StubType>*));

 private:
  AsyncStreamPtr stream_;
};

}  // namespace cpp2sky
