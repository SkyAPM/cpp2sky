load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "util_lib",
  hdrs = [
    "base64.h",
    "exception.h",
    "random_generator.h",
    "status.h",
  ],
  deps = [
    "//cpp2sky:random_interface",
    "@com_github_grpc_grpc//:grpc++",
  ],
  visibility = ["//visibility:public"],
)
