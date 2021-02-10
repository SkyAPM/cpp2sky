load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

licenses(["notice"])  # Apache 2

cc_library(
  name = "async_client_interface",
  hdrs = [
    "async_client.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "random_generator_interface",
  hdrs = [
    "random_generator.h",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cds_client_interface",
  hdrs = [
    "cds_client.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "reporter_client_interface",
  hdrs = [
    "reporter_client.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
  ],
  visibility = ["//visibility:public"],
)
