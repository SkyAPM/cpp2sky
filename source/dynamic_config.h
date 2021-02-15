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

#include "cpp2sky/config.pb.h"
#include "language-agent/ConfigurationDiscoveryService.pb.h"

namespace cpp2sky {

class DynamicConfig {
 public:
  DynamicConfig(TracerConfig& config) : config_(config) {}

  void onConfigChange(skywalking::v3::Commands commands) {
    std::cout << commands.DebugString() << std::endl;
    for (const auto& command: commands.commands()) {
      
    }
  }

  TracerConfig& tracerConfig() const { return config_; }

  const std::string& uuid() { return uuid_; }
  void updateUuid(std::string& uuid) { uuid_ = uuid; }

 private:
  TracerConfig& config_;
  std::string uuid_;
};

}  // namespace cpp2sky