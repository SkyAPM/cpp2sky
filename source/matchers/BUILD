load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "suffix_matcher_lib",
  hdrs = ["suffix_matcher.h"],
  srcs = ["suffix_matcher.cc"],
  deps = [
    "@com_github_abseil-cpp//absl/strings:strings",
    "//cpp2sky/internal:matcher_interface",
  ],
  visibility = ["//visibility:public"],
)
