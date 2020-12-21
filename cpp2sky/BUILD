load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

licenses(["notice"])  # Apache 2

proto_library(
  name = "config_proto_lib",
  srcs = ["config.proto"],
)

cc_proto_library(
  name = "config_cc_proto",
  deps = [":config_proto_lib"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_interface",
  hdrs = [
    "tracer.h",
    "segment_context.h",
    "propagation.h",
    "well_known_names.h",
    "exception.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    ":config_cc_proto",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_internal_interface",
  hdrs = [
    "internal/async_client.h",
    "internal/random_generator.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    ":config_cc_proto",
  ],
  visibility = ["//visibility:public"],
)
