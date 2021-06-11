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

#include <spdlog/logger.h>

#include <string_view>
#include <type_traits>

#include "cpp2sky/assert.h"

namespace cpp2sky {

static constexpr std::string_view SPDLOG_LOG_FORMAT =
    "{\"level\": \"%^%l%$\", \"msg\": \"%v";

template <class T>
std::string logFormat() {
  if constexpr (std::is_same_v<T, spdlog::logger>) {
    return SPDLOG_LOG_FORMAT.data();
  } else {
    CPP2SKY_STATIC_ASSERT(T, "non-supported logger type");
  }
}

}  // namespace cpp2sky
