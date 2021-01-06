load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "util_lib",
  hdrs = [
    "base64.h",
    "random_generator.h",
    "grpc_status.h",
    "circular_buffer.h",
  ],
  deps = [
    "//cpp2sky:cpp2sky_internal_interface",
  ],
  visibility = ["//visibility:public"],
)
