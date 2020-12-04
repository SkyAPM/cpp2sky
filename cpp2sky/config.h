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

class TracerConfig {
 public:
  TracerConfig(std::string address, Protocol protocol = Protocol::GRPC)
      : address_(address), protocol_(protocol) {}
  TracerConfig(std::string address, std::string token,
               Protocol protocol = Protocol::GRPC)
      : address_(address), token_(token), protocol_(protocol) {}
  ~TracerConfig() = default;

  const std::string& address() const { return address_; }
  const std::string& token() const { return token_; }
  Protocol protocol() const { return protocol_; }

 private:
  std::string address_;
  std::string token_;
  Protocol protocol_;
};

class SegmentConfig {
 public:
  SegmentConfig(std::string service_name, std::string instance_name)
      : service_name_(service_name), instance_name_(instance_name) {}

  const std::string& serviceName() const { return service_name_; }
  const std::string& instanceName() const { return instance_name_; }

 private:
  std::string service_name_;
  std::string instance_name_;
};

}  // namespace cpp2sky
