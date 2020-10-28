load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "util_lib",
  hdrs = [
    "base64.h",
    "exception.h",
    "random_generator.h",
  ],
  deps = [
    "//cpp2sky:random_interface",
  ],
  visibility = ["//visibility:public"],
)
