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
#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <memory>

using google::protobuf::Message;

namespace cpp2sky {

template <class RequestType, class ResponseType>
class StubWrapper {
 public:
  virtual ~StubWrapper() = default;

  /**
   * Initialize request writer.
   */
  virtual std::unique_ptr<grpc::ClientAsyncWriter<RequestType>> createWriter(
      grpc::ClientContext* ctx, ResponseType* response,
      grpc::CompletionQueue* cq, void* tag) = 0;
};

template <class RequestType, class ResponseType>
using StubWrapperPtr = std::shared_ptr<StubWrapper<RequestType, ResponseType>>;

template <class RequestType, class ResponseType>
class AsyncClient {
 public:
  virtual ~AsyncClient() = default;

  /**
   * Send the specified protobuf message
   */
  virtual void sendMessage(RequestType message) = 0;

  /**
   * Peer address of current gRPC client.
   */
  virtual std::string peerAddress() = 0;

  /**
   * Drain pending message.
   */
  virtual void drainPendingMessage(RequestType message) = 0;

  /**
   * Reset stream if it is living.
   */
  virtual void resetStream() = 0;

  /**
   * Start stream if there is no living stream.
   */
  virtual void startStream() = 0;

  /**
   * The number of drained pending messages.
   */
  virtual size_t numOfMessages() = 0;

  /**
   * Completion queue.
   */
  virtual grpc::CompletionQueue& completionQueue() = 0;

  /**
   * gRPC Stub
   */
  virtual StubWrapper<RequestType, ResponseType>& stub() = 0;
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
  Connected = 1,
  Idle = 2,
  WriteDone = 3,
};

class AsyncStreamCallback {
 public:
  /**
   * Callback when connected event occured.
   */ 
  virtual void onConnected() = 0;

  /**
   * Callback when idle event occured.
   */ 
  virtual void onIdle() = 0;

  /**
   * Callback when write done event occured.
   */ 
  virtual void onWriteDone() = 0;
};

struct StreamCallbackTag {
public:
  void callback() {
    switch (state_) {
      case StreamState::Connected: 
        callback_->onConnected();
        break;
      case StreamState::WriteDone:
        callback_->onWriteDone();
        break;
      case StreamState::Idle:
        callback_->onIdle();
        break;
    }
  }

  StreamState state_;
  AsyncStreamCallback* callback_;
};

template <class RequestType, class ResponseType>
using AsyncStreamPtr = std::shared_ptr<AsyncStream<RequestType, ResponseType>>;

template <class RequestType, class ResponseType>
class AsyncStreamFactory {
 public:
  virtual ~AsyncStreamFactory() = default;

  /**
   * Create async stream entity
   */
  virtual AsyncStreamPtr<RequestType, ResponseType> create(
      AsyncClient<RequestType, ResponseType>& client,
      std::condition_variable& cv) = 0;
};

template <class RequestType, class ResponseType>
using AsyncStreamFactoryPtr =
    std::unique_ptr<AsyncStreamFactory<RequestType, ResponseType>>;

}  // namespace cpp2sky
