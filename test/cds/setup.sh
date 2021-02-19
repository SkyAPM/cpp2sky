#!/bin/bash

curl http://localhost:8500/v1/kv/configuration-discovery.default.agentConfigurations
curl --request PUT --data-binary /tmp/config.yaml http://localhost:8500/v1/kv/configuration-discovery.default.agentConfigurations