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

#include "source/dynamic_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Common.pb.h"

namespace cpp2sky {

using testing::_;

skywalking::v3::Commands setCommands(std::string uuid_value,
                                     std::string instance_name) {
  skywalking::v3::Commands commands;
  auto* command = commands.add_commands();
  auto* uuid = command->add_args();
  uuid->set_key("UUID");
  uuid->set_value(uuid_value);
  auto* instance = command->add_args();
  instance->set_key("instance_name");
  instance->set_value(instance_name);
  return commands;
}

class DynamicConfigTest : public testing::Test {
 public:
  void setup() {
    init_cfg_.set_service_name("mesh");
    init_cfg_.set_instance_name("instance_default");

    dcfg_ = std::make_unique<DynamicConfig>(init_cfg_);
  }

  void update(skywalking::v3::Commands& commands) {
    prev_cfg_ = dcfg_->tracerConfig();
    prev_uuid_ = dcfg_->uuid();
    dcfg_->onConfigChange(commands);
  }

  void shouldChanged() {
    EXPECT_NE(dcfg_->tracerConfig().DebugString(), prev_cfg_.DebugString());
    EXPECT_NE(dcfg_->uuid(), prev_uuid_);
  }

  void shouldNotChanged() {
    EXPECT_EQ(dcfg_->tracerConfig().DebugString(), prev_cfg_.DebugString());
    EXPECT_EQ(dcfg_->uuid(), prev_uuid_);
  }

  TracerConfig init_cfg_;
  TracerConfig prev_cfg_;
  std::string prev_uuid_;
  std::unique_ptr<DynamicConfig> dcfg_;
};

TEST_F(DynamicConfigTest, ConfigChange) {
  setup();

  skywalking::v3::Commands commands1 = setCommands("uuid", "instance-updated");
  update(commands1);
  shouldChanged();

  // config not changed
  update(commands1);
  shouldNotChanged();

  // Config updated
  skywalking::v3::Commands commands2 =
      setCommands("uuid2", "instance-updated-2");
  update(commands2);
  shouldChanged();

  // UUID not set
  {
    skywalking::v3::Commands commands3;
    auto* command = commands3.add_commands();
    auto* instance = command->add_args();
    instance->set_key("instance_name");
    instance->set_value("instance-updated-3");

    update(commands3);
    shouldNotChanged();
  }

  // UUID updated but instance name doesn't changed with unknown field
  {
    skywalking::v3::Commands commands4;
    auto* command = commands4.add_commands();
    auto* uuid = command->add_args();
    uuid->set_key("UUID");
    uuid->set_value("uuid3");
    auto* unknown = command->add_args();
    unknown->set_key("unknown");
    unknown->set_value("instance-updated-3");

    update(commands4);
    shouldNotChanged();
  }

  // UUID upgraded and instance name changed, with unknown field addition
  {
    skywalking::v3::Commands commands5;
    auto* command = commands5.add_commands();
    auto* uuid = command->add_args();
    uuid->set_key("UUID");
    uuid->set_value("uuid4");
    auto* unknown = command->add_args();
    unknown->set_key("unknown");
    unknown->set_value("instance-updated-3");
    auto* instance = command->add_args();
    instance->set_key("instance_name");
    instance->set_value("instance-updated-3");

    update(commands5);
    shouldChanged();
  }
}

}  // namespace cpp2sky
