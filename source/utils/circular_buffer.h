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

#include <deque>
#include <mutex>

#include "absl/types/optional.h"

namespace cpp2sky {

template <class T>
class CircularBuffer {
 public:
  CircularBuffer(size_t max_capacity) : max_capacity_(max_capacity) {}

  // disable copy
  CircularBuffer(const CircularBuffer<T>&) = delete;
  CircularBuffer& operator=(const CircularBuffer<T>&) = delete;

  absl::optional<T> pop() {
    std::unique_lock<std::mutex> lock(mux_);
    if (buf_.empty()) {
      return {};
    }
    auto value = std::move(buf_.front());
    buf_.pop_front();
    return value;
  }

  void push(T value) {
    std::unique_lock<std::mutex> lock(mux_);
    if (buf_.size() >= max_capacity_) {
      buf_.pop_front();
    }
    buf_.emplace_back(value);
  }

  size_t size() {
    std::unique_lock<std::mutex> lock(mux_);
    return buf_.size();
  }

 private:
  size_t max_capacity_{};

  std::deque<T> buf_;
  std::mutex mux_;
};

}  // namespace cpp2sky
