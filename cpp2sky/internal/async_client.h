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

#include <functional>
#include <memory>

#include "language-agent/Tracing.pb.h"

namespace cpp2sky {

using TracerRequestType = skywalking::v3::SegmentObject;
using TracerResponseType = skywalking::v3::Commands;

template <class RequestType, class ResponseType>
class AsyncClientBase {
 public:
  virtual ~AsyncClientBase() = default;

  /**
   * Send the specified protobuf message.
   */
  virtual void sendMessage(RequestType message) = 0;

  virtual void resetClient() = 0;
};

using AsyncClient = AsyncClientBase<TracerRequestType, TracerResponseType>;
using AsyncClientPtr = std::unique_ptr<AsyncClient>;

template <class RequestType, class ResponseType>
class AsyncStreamBase {
 public:
  virtual ~AsyncStreamBase() = default;

  /**
   * Send the specified protobuf message.
   */
  virtual void sendMessage(RequestType message) = 0;
};

template <class RequestType, class ResponseType>
using AsyncStreamBasePtr =
    std::unique_ptr<AsyncStreamBase<RequestType, ResponseType>>;

using AsyncStream = AsyncStreamBase<TracerRequestType, TracerResponseType>;
using AsyncStreamSharedPtr = std::shared_ptr<AsyncStream>;

struct AsyncEventTag {
  std::function<void(bool)> callback;
};
using AsyncEventTagPtr = std::unique_ptr<AsyncEventTag>;

}  // namespace cpp2sky
