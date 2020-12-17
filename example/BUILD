load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "tracer_example",
  srcs = ["sample.cc"],
  deps = [
    "//source:cpp2sky_lib",
    "@com_github_httplib//:httplib",
  ],
)

cc_binary(
  name = "tracer_client_example",
  srcs = ["sample_client.cc"],
  deps = [
    "//source:cpp2sky_lib",
    "@com_github_httplib//:httplib",
  ],
)
