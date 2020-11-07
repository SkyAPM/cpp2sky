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
