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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "source/utils/circular_buffer.h"

namespace cpp2sky {

class CircularBufferTest : public testing::Test {
 protected:
  void setup(size_t size) {
    buf_ = std::make_unique<CircularBuffer<int>>(size);
  }

  void evaluate(size_t expect_front, size_t expect_back, bool expect_empty) {
    EXPECT_EQ(expect_front, buf_->frontIdx());
    EXPECT_EQ(expect_back, buf_->backIdx());
    EXPECT_EQ(expect_empty, buf_->empty());
  }

  void checkFront(int expect_value) {
    auto a = buf_->front();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(a.value(), expect_value);
  }

  std::unique_ptr<CircularBuffer<int>> buf_;
};

TEST_F(CircularBufferTest, Basic) {
  setup(3);
  for (auto i = 0; i < 1000; ++i) {
    buf_->pop();
  }

  buf_->push(1);
  buf_->push(2);
  buf_->push(3);
  evaluate(0, 2, false);

  buf_->push(4);
  evaluate(1, 0, false);

  buf_->push(5);
  buf_->push(6);
  evaluate(0, 2, false);

  checkFront(4);
  buf_->pop();
  evaluate(1, 2, false);

  checkFront(5);
  buf_->pop();
  evaluate(2, 2, false);

  buf_->push(7);
  evaluate(2, 0, false);

  checkFront(6);
  buf_->pop();
  evaluate(0, 0, false);

  checkFront(7);
  buf_->pop();
  // Return to Empty state
  evaluate(1, 0, true);

  buf_->push(8);
  evaluate(1, 1, false);

  buf_->push(9);
  buf_->push(10);
  buf_->push(11);
  buf_->push(12);

  checkFront(10);
  evaluate(0, 2, false);

  buf_->pop();
  buf_->pop();
  buf_->pop();

  evaluate(0, 2, true);

  for (auto i = 0; i < 1000; ++i) {
    buf_->pop();
  }

  evaluate(0, 2, true);
}

TEST_F(CircularBufferTest, Basic2) {
  setup(3);

  buf_->push(1);
  buf_->pop();
  evaluate(1, 0, true);

  buf_->push(2);
  evaluate(1, 1, false);

  buf_->push(3);
  buf_->push(4);

  buf_->pop();

  checkFront(3);
  buf_->pop();
  checkFront(4);
  buf_->pop();

  buf_->pop();
}

TEST_F(CircularBufferTest, Basic3) {
  setup(3);

  buf_->push(1);
  buf_->push(2);
  buf_->push(3);
  buf_->pop();
  buf_->push(4);
  buf_->push(5);

  evaluate(2, 1, false);
}

}  // namespace cpp2sky
