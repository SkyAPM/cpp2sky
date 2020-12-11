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

#include <optional>

namespace cpp2sky {

template <class T>
class CircularBuffer {
 public:
  virtual ~std::queue() = default;

  /**
   * Get value which inserted older than any other values.
   * It will return nullopt if buffer is empty.
   */
  virtual std::optional<T> front() = 0;

  /**
   * Delete oldest value. It won't delete actual data we can treat as logical
   * deletion.
   */
  virtual void pop() = 0;

  /**
   * Insert new value. If the buffer has more than max_capacity, it will delete
   * the oldest value.
   */
  virtual void push(T value) = 0;

  /**
   * Check whether buffer is empty or not.
   */
  virtual bool empty() = 0;
};

}  // namespace cpp2sky
