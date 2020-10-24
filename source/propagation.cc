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

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "source/utils/base64.h"
#include "source/utils/exception.h"

namespace cpp2sky {

namespace {
static constexpr size_t EXPECTED_FIELD_COUNT = 8;
}

SpanContext::SpanContext(std::string& header_value) {
  std::array<std::string, EXPECTED_FIELD_COUNT> fields;
  size_t current_field_idx = 0;
  std::string value;

  for (auto i = 0; i < header_value.size(); ++i) {
    if (current_field_idx >= EXPECTED_FIELD_COUNT) {
      throw TracerException(
          "Invalid span context format. It must have 8 fields.");
    }
    if (header_value[i] == '-') {
      fields[current_field_idx] = value;
      value.clear();
      ++current_field_idx;
      continue;
    }
    value += header_value[i];
  }
  fields[current_field_idx] = value;

  if (current_field_idx != EXPECTED_FIELD_COUNT - 1) {
    throw TracerException(
        "Invalid span context format. It must have 8 fields.");
  }

  if (fields[0] != "0" && fields[0] != "1") {
    throw TracerException(
        "Invalid span context format. sample field must be 0 or 1.");
  }

  must_send_ = fields[0] == "1" ? true : false;
  trace_id_ = Base64::decodeWithoutPadding(std::string_view(fields[1]));
  trace_segment_id_ = Base64::decodeWithoutPadding(std::string_view(fields[2]));
  span_id_ = std::stoi(fields[3]);
  service_ = Base64::decodeWithoutPadding(std::string_view(fields[4]));
  service_instance_ = Base64::decodeWithoutPadding(std::string_view(fields[5]));
  endpoint_ = Base64::decodeWithoutPadding(std::string_view(fields[6]));
  target_address_ = Base64::decodeWithoutPadding(std::string_view(fields[7]));
}

}  // namespace cpp2sky
