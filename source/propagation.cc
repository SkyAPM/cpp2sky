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

#include "source/propagation.h"

#include <cassert>
#include <iostream>
#include <string_view>

#include "source/utils/base64.h"

namespace cpp2sky {

SpanContext::SpanContext(std::string& header_value) {
  const size_t expected_field_count = 8;
  size_t current_field_idx = 0;
  size_t idx = 0;
  std::string value;

  while (expected_field_count != current_field_idx) {
    if (header_value[idx] == '-' || idx == header_value.size()) {
      if (current_field_idx == 0) {
        must_send_ = value == "1" ? true : false;
      } else if (current_field_idx == 1) {
        trace_id_ = Base64::decodeWithoutPadding(std::string_view(value));
      } else if (current_field_idx == 2) {
        trace_segment_id_ =
            Base64::decodeWithoutPadding(std::string_view(value));
      } else if (current_field_idx == 3) {
        span_id_ = std::stoi(value);
      } else if (current_field_idx == 4) {
        service_ = Base64::decodeWithoutPadding(std::string_view(value));
      } else if (current_field_idx == 5) {
        service_instance_ =
            Base64::decodeWithoutPadding(std::string_view(value));
      } else if (current_field_idx == 6) {
        endpoint_ = Base64::decodeWithoutPadding(std::string_view(value));
      } else if (current_field_idx == 7) {
        target_address_ = Base64::decodeWithoutPadding(std::string_view(value));
      } else {
        assert(false);
      }
      value.clear();
      ++current_field_idx;
      ++idx;
      continue;
    }
    value += header_value[idx];
    ++idx;
  }
}

}  // namespace cpp2sky