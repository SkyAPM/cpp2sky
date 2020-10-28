load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cpp2sky_dependencies():
  rules_proto()
  skywalking_data_collect_protocol()
  com_github_grpc_grpc()
  com_google_googletest()
  com_google_protobuf()

def skywalking_data_collect_protocol():
  http_archive(
    name = "skywalking_data_collect_protocol",
    sha256 = "ca495537bb85dbe8df5984ac7b571a0b87660281be69be1f9a0fa16fbf58f953",
    # TODO(shikugawa): Bazel upstreaming
    urls = ["https://github.com/Shikugawa/skywalking-data-collect-protocol/archive/v8.1.0-bazel.tar.gz"],
    strip_prefix = "skywalking-data-collect-protocol-8.1.0-bazel",
  )

def com_github_grpc_grpc():
  http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "3ccc4e5ae8c1ce844456e39cc11f1c991a7da74396faabe83d779836ef449bce",
    urls = ["https://github.com/grpc/grpc/archive/v1.27.0.tar.gz"],
    strip_prefix = "grpc-1.27.0",
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
