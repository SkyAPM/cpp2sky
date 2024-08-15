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

#include <functional>
#include <memory>

#include "google/protobuf/message.h"
#include "grpcpp/generic/generic_stub.h"
#include "grpcpp/grpcpp.h"
#include "language-agent/Tracing.pb.h"

namespace cpp2sky {

/**
 * Template base class for gRPC async client.
 */
template <class RequestType, class ResponseType>
class AsyncClientBase {
 public:
  virtual ~AsyncClientBase() = default;

  /**
   * Send the specified protobuf message.
   */
  virtual void sendMessage(RequestType message) = 0;

  /**
   * Reset the client. This should be called when the client is no longer
   * needed.
   */
  virtual void resetClient() = 0;
};

template <class RequestType, class ResponseType>
using AsyncClientBasePtr =
    std::unique_ptr<AsyncClientBase<RequestType, ResponseType>>;

/**
 * Template base class for gRPC async stream. The stream is used to represent
 * a single gRPC stream/request.
 */
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

/**
 * Tag for async operation. The callback should be called when the operation is
 * done.
 */
struct AsyncEventTag {
  std::function<void(bool)> callback;
};
using AsyncEventTagPtr = std::unique_ptr<AsyncEventTag>;

using GrpcClientContextPtr = std::unique_ptr<grpc::ClientContext>;
using GrpcCompletionQueue = grpc::CompletionQueue;

/**
 * Factory for creating async stream.
 */
template <class RequestType, class ResponseType>
class AsyncStreamFactoryBase {
 public:
  virtual ~AsyncStreamFactoryBase() = default;

  using AsyncStreamPtr = AsyncStreamBasePtr<RequestType, ResponseType>;
  using GrpcStub = grpc::TemplatedGenericStub<RequestType, ResponseType>;

  virtual AsyncStreamPtr createStream(GrpcClientContextPtr client_ctx,
                                      GrpcStub& stub, GrpcCompletionQueue& cq,
                                      AsyncEventTag& basic_event_tag,
                                      AsyncEventTag& write_event_tag) = 0;
};

template <class RequestType, class ResponseType>
using AsyncStreamFactoryBasePtr =
    std::unique_ptr<AsyncStreamFactoryBase<RequestType, ResponseType>>;

using TraceRequestType = skywalking::v3::SegmentObject;
using TraceResponseType = skywalking::v3::Commands;

using TraceAsyncStream = AsyncStreamBase<TraceRequestType, TraceResponseType>;
using TraceAsyncStreamPtr =
    AsyncStreamBasePtr<TraceRequestType, TraceResponseType>;

using TraceAsyncStreamFactory =
    AsyncStreamFactoryBase<TraceRequestType, TraceResponseType>;
using TraceAsyncStreamFactoryPtr =
    AsyncStreamFactoryBasePtr<TraceRequestType, TraceResponseType>;

using TraceAsyncClient = AsyncClientBase<TraceRequestType, TraceResponseType>;
using TraceAsyncClientPtr = std::unique_ptr<TraceAsyncClient>;

}  // namespace cpp2sky
