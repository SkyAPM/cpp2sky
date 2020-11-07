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

#include <string>

namespace cpp2sky {

enum Protocol { REST, GRPC };

class Config {
 public:
  Config(std::string service_name, std::string instance_name, std::string token,
         Protocol protocol = Protocol::GRPC)
      : service_name_(service_name),
        instance_name_(instance_name),
        token_(token),
        protocol_(protocol) {}
  ~Config() = default;

  const std::string& serviceName() const { return service_name_; }
  const std::string& instanceName() const { return instance_name_; }
  const std::string& token() const { return token_; }
  Protocol protocol() const { return protocol_; }

 private:
  std::string service_name_;
  std::string instance_name_;
  std::string token_;
  Protocol protocol_;
};

}  // namespace cpp2sky
