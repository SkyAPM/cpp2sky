load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "util_lib",
  hdrs = [
    "base64.h",
    "random_generator.h",
    "circular_buffer.h",
    "timer.h",
  ],
  srcs = [
    "random_generator.cc",
  ],
  deps = [
    "//cpp2sky/internal:random_generator_interface",
  ],
  visibility = ["//visibility:public"],
)
