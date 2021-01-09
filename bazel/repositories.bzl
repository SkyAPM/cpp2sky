load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cpp2sky_dependencies():
  rules_proto()
  skywalking_data_collect_protocol()
  com_github_grpc_grpc()
  com_google_googletest()
  com_google_protobuf()
  com_github_httplib()

def skywalking_data_collect_protocol():
  http_archive(
    name = "skywalking_data_collect_protocol",
    sha256 = "8158e095b9b37c39e18938fe08bd4fca6f2b8e16763ff21fe8118d79241a6e0b",
    urls = ["https://github.com/apache/skywalking-data-collect-protocol/archive/v8.3.0.tar.gz"],
    strip_prefix = "skywalking-data-collect-protocol-8.3.0",
  )

def com_github_grpc_grpc():
  http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "06a87c5feb7efb979243c054dca2ea52695618c02fde54af8a85d71269f97102",
    urls = ["https://github.com/grpc/grpc/archive/v1.33.0.tar.gz"],
    strip_prefix = "grpc-1.33.0",
  )

def rules_proto():
  http_archive(
    name = "rules_proto",
    sha256 = "602e7161d9195e50246177e7c55b2f39950a9cf7366f74ed5f22fd45750cd208",
    strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz",
    ],
  )

def com_google_googletest():
  http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.tar.gz"],
  )

def com_google_protobuf():
  http_archive(
    name = "com_google_protobuf",
    sha256 = "f8a547dfe143a9f61fadafba47fa6573713a33cb80909307c1502e26e1102298",
    strip_prefix = "protobuf-3.13.0",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.13.0/protobuf-cpp-3.13.0.tar.gz"],
  )

def com_github_httplib():
  http_archive(
    name = "com_github_httplib",
    sha256 = "0e424f92b607fc9245c144dada85c2e97bc6cc5938c0c69a598a5b2a5c1ab98a",
    strip_prefix = "cpp-httplib-0.7.15",
    build_file = "//bazel:httplib.BUILD",
    urls = ["https://github.com/yhirose/cpp-httplib/archive/v0.7.15.tar.gz"]
  )
