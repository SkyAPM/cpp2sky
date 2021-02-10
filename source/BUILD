load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "cpp2sky_lib",
  hdrs = [
    "propagation_impl.h",
    "tracing_context_impl.h",
    "grpc_async_client_impl.h",
    "tracer_impl.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "tracing_context_impl.cc",
    "grpc_async_client_impl.cc",
    "tracer_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
    "//cpp2sky/internal:async_client_interface",
    "//cpp2sky/internal:stream_builder_interface",
    "//cpp2sky:cpp2sky_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)

# TODO(shikugawa): This is a terrible hack to disable unexpected behavior for 
# test that has introduced GRPC_TEST.
cc_library(
  name = "async_client_impl_test",
  hdrs = [
    "grpc_async_client_impl.h",
  ],
  srcs = [
    "grpc_async_client_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
    "//cpp2sky/internal:async_client_interface",
    "//cpp2sky/internal:stream_builder_interface",
    "//cpp2sky:cpp2sky_interface",
    "//source/utils:util_lib",
  ],
  copts = ["-DGRPC_TEST"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "cpp2sky_data_lib",
  hdrs  =[
    "propagation_impl.h",
    "tracing_context_impl.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "tracing_context_impl.cc",
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "//cpp2sky:config_cc_proto",
    "//cpp2sky:cpp2sky_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)
