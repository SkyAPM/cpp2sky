load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "e2e_consumer",
  srcs = ["consumer.cc"],
  deps = [
    "//source:cpp2sky_lib",
    "@com_github_httplib//:httplib",
  ],
)

cc_binary(
  name = "e2e_provider",
  srcs = ["provider.cc"],
  deps = [
    "//source:cpp2sky_lib",
    "@com_github_httplib//:httplib",
  ],
)
