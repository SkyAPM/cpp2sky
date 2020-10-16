load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "tracer_example",
  srcs = ["sample.cc"],
  deps = [
    "//cpp2sky:tracer_interface",
    "//source:tracer_lib",
  ],
  visibility = ["//visibility:public"],
)
