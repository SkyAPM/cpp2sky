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

#include "dynamic_config.h"

#include <string>

#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "spdlog/spdlog.h"

namespace cpp2sky {

namespace {  // well known fields on response commands.
static constexpr std::string_view UUID_FIELD = "UUID";
static constexpr std::string_view SERIAL_NUMBER_FIELD = "SerialNumber";
static constexpr std::string_view IGNORE_SUFFIX = "ignore_suffix";
}  // namespace

using namespace spdlog;

DynamicConfig::DynamicConfig(TracerConfig& config) : config_(config) {
  target_fields_.emplace(IGNORE_SUFFIX.data());

  ignore_fields_.emplace(UUID_FIELD.data());
  ignore_fields_.emplace(SERIAL_NUMBER_FIELD.data());
}

void DynamicConfig::onConfigChange(skywalking::v3::Commands commands) {
  if (commands.commands_size() <= 0) {
    return;
  }
  const auto top_command = commands.commands(0);

  std::string uuid;
  for (const auto& target : top_command.args()) {
    if (target.key() == UUID_FIELD.data()) {
      uuid = target.value();
    }
  }

  if (uuid.empty() || uuid_ == uuid) {
    info("UUID not changed changed from {}", uuid_);
    return;
  }

  std::unique_lock<std::mutex> lck(mux_);
  bool config_changed = false;
  for (const auto& target : top_command.args()) {
    if (ignore_fields_.find(target.key()) != ignore_fields_.end() ||
        target_fields_.find(target.key()) == target_fields_.end()) {
      continue;
    }

    if (target.key() == IGNORE_SUFFIX) {
      std::string will_destroy_suffixes;
      for (const auto& current_suffix :
           config_.ignore_operation_name_suffix()) {
        will_destroy_suffixes += current_suffix + ",";
      }
      will_destroy_suffixes = absl::StripSuffix(will_destroy_suffixes, ",");

      config_.clear_ignore_operation_name_suffix();

      for (const auto& next_suffix : absl::StrSplit(target.value(), ",")) {
        *config_.add_ignore_operation_name_suffix() = next_suffix;
      }

      info("{} updated from {} to {}", IGNORE_SUFFIX.data(),
           will_destroy_suffixes, target.value());
    }

    config_changed = true;
  }

  if (config_changed) {
    uuid_ = uuid;
  }
}

}  // namespace cpp2sky
