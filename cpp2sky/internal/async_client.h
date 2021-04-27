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
   * Pending message queue reference.
   */
  virtual CircularBuffer<RequestType>& pendingMessages() = 0;

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
   * Send message. It will move the state from Init to Write.
   */
  virtual void sendMessage(RequestType message) = 0;
};

enum class StreamState : uint8_t {
  Initialized = 0,
  Ready = 1,
  Idle = 2,
  WriteDone = 3,
  ReadDone = 4,
};

class AsyncStreamCallback {
 public:
  /**
   * Callback when stream ready event occured.
   */
  virtual void onReady() = 0;

  /**
   * Callback when idle event occured.
   */
  virtual void onIdle() = 0;

  /**
   * Callback when write done event occured.
   */
  virtual void onWriteDone() = 0;

  /**
   * Callback when read done event occured.
   */
  virtual void onReadDone() = 0;

  /**
   * Callback when stream had finished with arbitrary error.
   */
  virtual void onStreamFinish() = 0;
};

struct StreamCallbackTag {
 public:
  void callback(bool stream_finished) {
    if (stream_finished) {
      callback_->onStreamFinish();
      return;
    }

    switch (state_) {
      case StreamState::Ready:
        callback_->onReady();
        break;
      case StreamState::WriteDone:
        callback_->onWriteDone();
        break;
      case StreamState::Idle:
        callback_->onIdle();
        break;
      case StreamState::ReadDone:
        callback_->onReadDone();
        break;
      default:
        break;
    }
  }

  StreamState state_;
  AsyncStreamCallback* callback_;
};

template <class RequestType, class ResponseType>
using AsyncStreamPtr = std::shared_ptr<AsyncStream<RequestType, ResponseType>>;

}  // namespace cpp2sky
