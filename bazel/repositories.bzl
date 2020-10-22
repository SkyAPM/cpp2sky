load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cpp2sky_dependencies():
  skywalking_data_collect_protocol()
  com_github_grpc_grpc()

def skywalking_data_collect_protocol():
  http_archive(
    name = "skywalking_data_collect_protocol",
    sha256 = "e845bbbe15053f5ed4bf085da183256fc448e2650e3f7cffb35206d3e8ed6c92",
    # TODO(shikugawa): Bazel upstreaming
    urls = ["https://github.com/Shikugawa/skywalking-data-collect-protocol/archive/v8.1.0-bazel.tar.gz"],
    strip_prefix = "skywalking-data-collect-protocol-8.1.0-bazel",
  )

def com_github_grpc_grpc():
  http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "58eaee5c0f1bd0b92ebe1fa0606ec8f14798500620e7444726afcaf65041cb63",
    urls = ["https://github.com/grpc/grpc/archive/1.33.1.tar.gz"],
    strip_prefix = "grpc-1.33.1",
  )