load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "cpp2sky_lib",
  hdrs = [
    "propagation_impl.h",
    "segment_context_impl.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "segment_context_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "//cpp2sky:config_interface",
    "//cpp2sky:segment_context_interface",
    "//cpp2sky:propagation_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)
