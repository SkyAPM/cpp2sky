// Copyright 2021 SkyAPM

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

#include <grpcpp/grpcpp.h>

#include <condition_variable>

namespace cpp2sky {

template <class RequestType, class ResponseType>
class ClientStreamingStreamBuilder {
 public:
  virtual ~ClientStreamingStreamBuilder() = default;

  /**
   * Create async stream entity
   */
  virtual AsyncStreamPtr<RequestType, ResponseType> create(
      AsyncClient<RequestType, ResponseType>& client,
      std::condition_variable& cv) = 0;
};

template <class RequestType, class ResponseType>
using ClientStreamingStreamBuilderPtr =
    std::unique_ptr<ClientStreamingStreamBuilder<RequestType, ResponseType>>;

template <class RequestType, class ResponseType>
class UnaryStreamBuilder {
 public:
  virtual ~UnaryStreamBuilder() = default;

  /**
   * Create async stream entity
   */
  virtual AsyncStreamPtr<RequestType, ResponseType> create(
      AsyncClient<RequestType, ResponseType>& client, RequestType request) = 0;
};

template <class RequestType, class ResponseType>
using UnaryStreamBuilderPtr =
    std::unique_ptr<UnaryStreamBuilder<RequestType, ResponseType>>;

}  // namespace cpp2sky
