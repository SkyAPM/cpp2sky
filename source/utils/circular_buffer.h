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

#include "cpp2sky/internal/circular_buffer.h"

namespace cpp2sky {

template <class T>
class CircularBufferImpl : public CircularBuffer<T> {
 public:
  CircularBufferImpl(size_t max_capacity) : max_capacity_(max_capacity) {}

  // disable copy
  CircularBufferImpl(const CircularBufferImpl<T>&) = delete;
  CircularBufferImpl& operator=(const CircularBufferImpl<T>&) = delete;

  struct Buffer {
    T value;
    bool is_destroyed_ = false;
  };

  std::optional<T> front() override {
    if (empty()) {
      return std::nullopt;
    }
    return buf_[front_].value;
  }

  void pop() override {
    std::unique_lock<std::mutex> lock(mux_);
    popInternal();
  }

  void push(T value) override {
    std::unique_lock<std::mutex> lock(mux_);
    if (buf_.size() < max_capacity_) {
      buf_.emplace_back(Buffer{value, false});
      back_ = buf_.size() - 1;
      ++item_count_;
      return;
    }

    back_ = (back_ + 1) % max_capacity_;
    if (!buf_[back_].is_destroyed_) {
      popInternal();
    }
    buf_[back_] = Buffer{value, false};
    ++item_count_;
  }

  bool empty() override { return item_count_ == 0; }

  // Used for test
#pragma region test
  size_t frontIdx() { return front_; }
  size_t backIdx() { return back_; }
#pragma endregion

 private:
  void popInternal() {
    if (buf_[front_].is_destroyed_) {
      return;
    }

    // Not to destroy actual data.
    buf_[front_].is_destroyed_ = true;
    --item_count_;
    front_ = (front_ + 1) % max_capacity_;
  }

  size_t front_ = 0;
  size_t back_ = 0;
  size_t max_capacity_;
  size_t item_count_ = 0;

  std::deque<Buffer> buf_;
  std::mutex mux_;
};

}  // namespace cpp2sky
