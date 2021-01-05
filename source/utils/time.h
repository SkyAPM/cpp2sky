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

#include <chrono>

namespace cpp2sky {

using SystemTime = std::chrono::system_clock;
using SystemTimePoint = SystemTime::time_point;
;

int64_t millisecondsFromEpoch(SystemTimePoint time) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             time.time_since_epoch())
      .count();
}

}  // namespace cpp2sky
