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
    "utils/base64.h",
  ],
  srcs = ["propagation.cc"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "segment_context_lib",
  hdrs = [
    "segment_context.h",
    "utils/random_generator.h",
  ],
  srcs = ["segment_context.cc"],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    ":propagation_lib",
    ":config_lib",
  ],
  visibility = ["//visibility:public"],
)
