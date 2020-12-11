load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "config_interface",
  hdrs = ["config.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "random_interface",
  hdrs = ["internal/random_generator.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "buffer_interface",
  hdrs = ["internal/circular_buffer.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "propagation_interface",
  hdrs = ["propagation.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "segment_context_interface",
  hdrs = ["segment_context.h"],
  deps = [
    ":config_interface",
    ":propagation_interface",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "async_client_interface",
  hdrs = ["internal/async_client.h"],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@com_google_protobuf//:protobuf",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "tracer_interface",
  hdrs = ["tracer.h"],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_interface",
  hdrs = [
    "tracer.h",
    "segment_context.h",
    "propagation.h",
    "config.h",
  ],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
  ],
  visibility = ["//visibility:public"],
) 
