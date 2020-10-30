load("@rules_cc//cc:defs.bzl", "cc_test", "cc_library")

cc_library(
  name = "mocks",
  srcs = [
    "mocks.h",
  ],
  deps = [
    "@com_google_googletest//:gtest_main",
    "//cpp2sky:async_client_interface",
    "//cpp2sky:random_interface",
  ]
)

cc_test(
  name = "propagation_test",
  srcs = [
    "propagation_test.cc",
  ],
  deps = [
    "@com_google_googletest//:gtest_main",
    "//source:propagation_lib",
  ],
  visibility = ["//visibility:public"],
)

cc_test(
  name = "segment_context_test",
  srcs = [
    "segment_context_test.cc",
  ],
  deps = [
    "@com_google_googletest//:gtest_main",
    "@com_google_protobuf//:protobuf",
    "//source:propagation_lib",
    "//source:segment_context_lib",
    "//source/utils:util_lib",
    ":mocks",
  ],
  visibility = ["//visibility:public"],
)

cc_test(
  name = "grpc_async_test",
  srcs = [
    "grpc_async_test.cc",
  ],
  deps = [
    "@com_google_googletest//:gtest_main",
    "//source:grpc_async_client_lib",
    ":mocks",
  ],
  visibility = ["//visibility:public"],
)
