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

#include <condition_variable>

#include "cpp2sky/internal/async_client.h"
#include "cpp2sky/internal/random_generator.h"
#include "cpp2sky/internal/stream_builder.h"

using testing::_;
using testing::Return;

namespace cpp2sky {

class MockRandomGenerator : public RandomGenerator {
 public:
  MockRandomGenerator() { ON_CALL(*this, uuid).WillByDefault(Return("uuid")); }
  MOCK_METHOD(std::string, uuid, ());
};

template <class RequestType, class ResponseType>
class MockAsyncStream : public AsyncStream<RequestType, ResponseType> {
 public:
  MOCK_METHOD(void, sendMessage, (RequestType));
  MOCK_METHOD(void, onIdle, ());
  MOCK_METHOD(void, onWriteDone, ());
  MOCK_METHOD(void, onReady, ());
};

template <class RequestType, class ResponseType>
class MockAsyncClient : public AsyncClient<RequestType, ResponseType> {
 public:
  using GenericStub = grpc::TemplatedGenericStub<RequestType, ResponseType>;

  MOCK_METHOD(void, sendMessage, (RequestType));
  MOCK_METHOD(GenericStub&, stub, ());
  MOCK_METHOD(CircularBuffer<RequestType>&, pendingMessages, ());
  MOCK_METHOD(void, startStream, ());
  MOCK_METHOD(grpc::CompletionQueue&, completionQueue, ());
};

template <class RequestType, class ResponseType>
class MockClientStreamingStreamBuilder final
    : public ClientStreamingStreamBuilder<RequestType, ResponseType> {
 public:
  using AsyncClientType = AsyncClient<RequestType, ResponseType>;
  using AsyncStreamPtrType = AsyncStreamPtr<RequestType, ResponseType>;

  MockClientStreamingStreamBuilder(
      std::shared_ptr<MockAsyncStream<RequestType, ResponseType>> stream)
      : stream_(stream) {
    ON_CALL(*this, create(_, _)).WillByDefault(Return(stream_));
  }

  MOCK_METHOD(AsyncStreamPtrType, create,
              (AsyncClientType&, std::condition_variable&));

 private:
  std::shared_ptr<MockAsyncStream<RequestType, ResponseType>> stream_;
};

}  // namespace cpp2sky
