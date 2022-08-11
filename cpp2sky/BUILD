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
    "tracing_context.h",
    "propagation.h",
    "well_known_names.h",
    "exception.h",
    "time.h",
    "assert.h",
    "trace_log.h",
  ],
  deps = [
    ":config_cc_proto",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_data_interface",
  hdrs = [
    "tracing_context.h",
    "propagation.h",
    "well_known_names.h",
    "exception.h",
    "time.h",
    "assert.h",
  ],
  deps = [
    ":config_cc_proto",
    "@com_google_absl//absl/memory:memory",
    "@com_google_absl//absl/strings:strings",
    "@com_google_absl//absl/types:optional",
  ],
  visibility = ["//visibility:public"],
)
