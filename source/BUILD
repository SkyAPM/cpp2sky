load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "config_lib",
  hdrs = ["config_impl.h"],
  deps = [
    "//cpp2sky:config_interface",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "propagation_lib",
  hdrs = [
    "propagation.h",
  ],
  srcs = ["propagation.cc"],
  deps = [
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "segment_context_lib",
  hdrs = [
    "segment_context.h",
  ],
  srcs = ["segment_context.cc"],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    ":propagation_lib",
    ":config_lib",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "grpc_async_client_lib",
  hdrs = [
    "grpc_async_client.h",
  ],
  srcs = ["grpc_async_client.cc"],
  deps = [
    "//cpp2sky:async_client_interface",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)
