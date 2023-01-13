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

#include <google/protobuf/message.h>
#include <grpcpp/generic/generic_stub.h>
#include <grpcpp/grpcpp.h>

#include <memory>

#include "source/utils/circular_buffer.h"

using google::protobuf::Message;

namespace cpp2sky {

template <class RequestType, class ResponseType>
class AsyncClient {
 public:
  virtual ~AsyncClient() = default;

  /**
   * Send the specified protobuf message.
   */
  virtual void sendMessage(RequestType message) = 0;

  /**
   * Start stream if there is no living stream.
   */
  virtual void startStream() = 0;

  /**
   * Completion queue.
   */
  virtual grpc::CompletionQueue& completionQueue() = 0;

  /**
   * gRPC Stub
   */
  virtual grpc::TemplatedGenericStub<RequestType, ResponseType>& stub() = 0;
};

template <class RequestType, class ResponseType>
using AsyncClientPtr = std::unique_ptr<AsyncClient<RequestType, ResponseType>>;

template <class RequestType, class ResponseType>
class AsyncStream {
 public:
  virtual ~AsyncStream() = default;

  /**
   * Send messages to server.
   */
  virtual void sendMessage(RequestType message) = 0;
};

class StreamCallbackTag {
 public:
  StreamCallbackTag() = default;

  StreamCallbackTag(std::function<bool(bool)> cb) : cb_{std::move(cb)} {}

  /**
   * @return continue event loop or not.
   */
  bool callback(bool event_ok) {
    if (cb_ != nullptr) {
      return cb_(event_ok);
    }
    return true;
  }

 private:
  std::function<bool(bool)> cb_;
};

template <class RequestType, class ResponseType>
using AsyncStreamSharedPtr =
    std::shared_ptr<AsyncStream<RequestType, ResponseType>>;

}  // namespace cpp2sky
