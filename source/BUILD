load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
  name = "cpp2sky_lib",
  hdrs = [
    "propagation_impl.h",
    "tracing_context_impl.h",
    "grpc_async_client_impl.h",
    "tracer_impl.h",
    "cds_impl.h",
    "dynamic_config.h",
  ],
  srcs = [
    "propagation_impl.cc",
    "tracing_context_impl.cc",
    "grpc_async_client_impl.cc",
    "tracer_impl.cc",
    "cds_impl.cc",
    "dynamic_config.cc"
  ],
  deps = [
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_proto",
    "@skywalking_data_collect_protocol//language-agent:tracing_protocol_cc_grpc",
    "@skywalking_data_collect_protocol//language-agent:configuration_discovery_service_cc_proto",
    "@skywalking_data_collect_protocol//language-agent:configuration_discovery_service_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
    "@com_github_gabime_spdlog//:spdlog",
    "@com_google_absl//absl/strings:strings",
    "//cpp2sky/internal:async_client_interface",
    "//cpp2sky/internal:stream_builder_interface",
    "//cpp2sky/internal:matcher_interface",
    "//cpp2sky:cpp2sky_interface",
    "//source/utils:util_lib",
    "//source/matchers:suffix_matcher_lib",
  ],
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
    "//cpp2sky:cpp2sky_data_interface",
    "//source/utils:util_lib",
  ],
  visibility = ["//visibility:public"],
)
