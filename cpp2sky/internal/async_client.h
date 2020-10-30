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

using google::protobuf::Message;

namespace cpp2sky {

class AsyncClient {
 public:
  virtual ~AsyncClient() = default;

  /**
   * Send the specified protobuf message
   */
  virtual void onSendMessage(const Message& message) = 0;
};

using AsyncClientPtr = std::unique_ptr<AsyncClient>;

class AsyncStream {
 public:
  virtual ~AsyncStream() = default;

  /**
   * Get response status code. grpc-status code will be translated to generic
   * status code.
   */
  virtual uint16_t status() const = 0;

  /**
   * Establish stream.
   */
  virtual void startStream() = 0;

  /**
   * Set data per stream.
   */
  virtual void setData(const Message& message) = 0;

  /**
   * Get response message.
   */
  virtual const Message& reply() const = 0;
};

using AsyncStreamPtr = std::shared_ptr<AsyncStream>;

}  // namespace cpp2sky
