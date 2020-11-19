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

#include <memory>

using google::protobuf::Message;

namespace cpp2sky {

template <class Stub>
class AsyncClient {
 public:
  virtual ~AsyncClient() = default;

  /**
   * Send the specified protobuf message
   */
  virtual bool sendMessage(Message& message) = 0;

  /**
   * Get queue to execute gRPC async stream dispatching.
   */
  virtual grpc::CompletionQueue* completionQueue() = 0;

  /**
   * Get context for gRPC client.
   */
  virtual grpc::ClientContext* grpcClientContext() = 0;

  /**
   * Get stub.
   */
  virtual Stub* grpcStub() = 0;
};

template <class T>
using AsyncClientPtr = std::unique_ptr<AsyncClient<T>>;

class AsyncStream {
 public:
  virtual ~AsyncStream() = default;

  /**
   * Start stream. It will move the state of stream to Init.
   */
  virtual bool startStream() = 0;

  /**
   * Send message. It will move the state from Init to Write.
   */
  virtual bool sendMessage(Message& message) = 0;

  /**
   * Finish to write on this stream. It will move the state from Write to
   * WriteDone.
   */
  virtual bool writeDone() = 0;
};

using AsyncStreamPtr = std::shared_ptr<AsyncStream>;

template <class Stub>
class AsyncStreamFactory {
 public:
  virtual ~AsyncStreamFactory() = default;

  /**
   * Create async stream entity
   */
  virtual AsyncStreamPtr create(AsyncClient<Stub>* client) = 0;
};

template <class T>
using AsyncStreamFactoryPtr = std::unique_ptr<AsyncStreamFactory<T>>;

}  // namespace cpp2sky
