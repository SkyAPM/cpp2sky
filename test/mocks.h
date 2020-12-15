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

using testing::_;
using testing::Return;

namespace cpp2sky {

class MockRandomGenerator : public RandomGenerator {
 public:
  MockRandomGenerator() { ON_CALL(*this, uuid).WillByDefault(Return("uuid")); }
  MOCK_METHOD(std::string, uuid, ());
};

template <class RequestType>
class MockAsyncStream : public AsyncStream<RequestType> {
 public:
  MOCK_METHOD(void, startStream, ());
  MOCK_METHOD(void, sendMessage, (RequestType));
  MOCK_METHOD(std::string, peerAddress, ());
  MOCK_METHOD(bool, handleOperation, (Operation));
  MOCK_METHOD(void, undrainMessage, (RequestType));
};

template <class RequestType, class ResponseType>
class MockAsyncClient : public AsyncClient<RequestType, ResponseType> {
 public:
  MOCK_METHOD(void, sendMessage, (RequestType));
  MOCK_METHOD(std::unique_ptr<grpc::ClientAsyncWriter<RequestType>>,
              createWriter, (grpc::ClientContext*, ResponseType*, void*));
  MOCK_METHOD(std::string, peerAddress, ());
  MOCK_METHOD(void, drainPendingMessage, (RequestType));
  MOCK_METHOD(void, resetStream, ());
  MOCK_METHOD(bool, startStream, ());
  MOCK_METHOD(size_t, numOfMessages, ());
};

template <class RequestType, class ResponseType>
class MockAsyncStreamFactory
    : public AsyncStreamFactory<RequestType, ResponseType> {
 public:
  using AsyncClientType = AsyncClient<RequestType, ResponseType>;
  MockAsyncStreamFactory(AsyncStreamPtr<RequestType> stream) : stream_(stream) {
    ON_CALL(*this, create(_, _)).WillByDefault(Return(stream_));
  }
  MOCK_METHOD(AsyncStreamPtr<RequestType>, create,
              (AsyncClientType*, std::condition_variable&));
  MOCK_METHOD(void, setPendingBufferSize, (uint64_t));

 private:
  AsyncStreamPtr<RequestType> stream_;
};

}  // namespace cpp2sky
