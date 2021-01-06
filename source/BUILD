load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "cpp2sky_lib",
  hdrs = [
    "propagation_impl.h",
    "segment_context_impl.h",
    "grpc_async_client_impl.h",
    "tracer_impl.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "segment_context_impl.cc",
    "grpc_async_client_impl.cc",
    "tracer_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
    "//cpp2sky:cpp2sky_interface",
    "//cpp2sky:cpp2sky_internal_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_data_lib",
  hdrs  =[
    "propagation_impl.h",
    "segment_context_impl.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "segment_context_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "//cpp2sky:config_cc_proto",
    "//cpp2sky:cpp2sky_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)
