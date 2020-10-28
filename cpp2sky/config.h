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
  virtual ~Config() = default;

  /**
   * global service name.
   */
  virtual const std::string& serviceName() const = 0;

  /**
   * Instance name belongs to service.
   */
  virtual const std::string& instanceName() const = 0;

  /**
   * Protocol to communicate between app and OAP.
   * It supports only GRPC. (REST is in the future)
   */
  virtual Protocol protocol() const = 0;

  /**
   * OAP token.
   */
  virtual const std::string& token() const = 0;
};

}  // namespace cpp2sky
