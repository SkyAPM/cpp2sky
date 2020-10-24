load("@rules_cc//cc:defs.bzl", "cc_test")

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
