load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "tracer_lib",
  srcs = ["tracer_impl.cc"],
  hdrs = ["tracer_impl.h"],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "@com_github_grpc_grpc//:grpc++",
    "//sky2cpp:tracer_interface"
  ],
  visibility = ["//visibility:public"],
)
