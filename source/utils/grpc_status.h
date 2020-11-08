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
//

#pragma once

#include <grpcpp/grpcpp.h>

namespace cpp2sky {

// Based on:
// https://github.com/envoyproxy/envoy/blob/master/source/common/grpc/status.cc
uint16_t grpcStatusToGenericHttpStatus(grpc::StatusCode status) {
  switch (status) {
    case grpc::StatusCode::OK:
      return 200;
    case grpc::StatusCode::CANCELLED:
      // Client closed request.
      return 499;
    case grpc::StatusCode::UNKNOWN:
      // Internal server error.
      return 500;
    case grpc::StatusCode::INVALID_ARGUMENT:
      // Bad request.
      return 400;
    case grpc::StatusCode::DEADLINE_EXCEEDED:
      // Gateway Time-out.
      return 504;
    case grpc::StatusCode::NOT_FOUND:
      // Not found.
      return 404;
    case grpc::StatusCode::ALREADY_EXISTS:
      // Conflict.
      return 409;
    case grpc::StatusCode::PERMISSION_DENIED:
      // Forbidden.
      return 403;
    case grpc::StatusCode::RESOURCE_EXHAUSTED:
      //  Too many requests.
      return 429;
    case grpc::StatusCode::FAILED_PRECONDITION:
      // Bad request.
      return 400;
    case grpc::StatusCode::ABORTED:
      // Conflict.
      return 409;
    case grpc::StatusCode::OUT_OF_RANGE:
      // Bad request.
      return 400;
    case grpc::StatusCode::UNIMPLEMENTED:
      // Not implemented.
      return 501;
    case grpc::StatusCode::INTERNAL:
      // Internal server error.
      return 500;
    case grpc::StatusCode::UNAVAILABLE:
      // Service unavailable.
      return 503;
    case grpc::StatusCode::DATA_LOSS:
      // Internal server error.
      return 500;
    case grpc::StatusCode::UNAUTHENTICATED:
      // Unauthorized.
      return 401;
    default:
      // Internal server error.
      return 500;
  }
}

}  // namespace cpp2sky
