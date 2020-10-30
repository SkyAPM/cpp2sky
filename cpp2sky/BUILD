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
  name = "async_client_interface",
  hdrs = ["internal/async_client.h"],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@com_google_protobuf//:protobuf",
  ],
  visibility = ["//visibility:public"],
)
