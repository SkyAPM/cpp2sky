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

#include <mutex>
#include <set>
#include <string_view>

#include "cpp2sky/config.pb.h"
#include "language-agent/ConfigurationDiscoveryService.pb.h"

namespace cpp2sky {

class DynamicConfig {
 public:
  DynamicConfig(TracerConfig& config);

  void onConfigChange(skywalking::v3::Commands commands);
  const TracerConfig& tracerConfig() const& {
    std::unique_lock<std::mutex> lck(mux_);
    return config_;
  }
  const std::string& uuid() const { return uuid_; }

 private:
  mutable std::mutex mux_;
  TracerConfig& config_;
  std::string uuid_;
  std::set<std::string> target_fields_;
  std::set<std::string> ignore_fields_;
};

}  // namespace cpp2sky
