load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cpp2sky_dependencies():
  rules_proto()
  skywalking_data_collect_protocol()
  com_github_grpc_grpc()
  com_google_googletest()
  com_google_protobuf()
  com_github_httplib()
  com_github_fmtlib_fmt()
  com_google_abseil()
  com_github_gabime_spdlog()

def skywalking_data_collect_protocol():
  http_archive(
    name = "skywalking_data_collect_protocol",
    sha256 = "49bd689b9c1c0ea12064bd35581689cef7835e5ac15d335dc425fbfc2029aa90",
    urls = [
      "https://github.com/apache/skywalking-data-collect-protocol/archive/v8.9.1.tar.gz"
    ],
    strip_prefix = "skywalking-data-collect-protocol-8.9.1",
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

def com_github_fmtlib_fmt():
  http_archive(
    name = "com_github_fmtlib_fmt",
    sha256 = "decfdf9ad274070fa85f26407b816f5a4d82205ae86bac1990be658d0795ea4d",
    strip_prefix = "fmt-7.0.3",
    build_file = "//bazel:fmtlib.BUILD",
    urls = ["https://github.com/fmtlib/fmt/releases/download/7.0.3/fmt-7.0.3.zip"],
  )

def com_github_gabime_spdlog():
  http_archive(
    name = "com_github_gabime_spdlog",
    sha256 = "f0114a4d3c88be9e696762f37a7c379619443ce9d668546c61b21d41affe5b62",
    strip_prefix = "spdlog-1.7.0",
    build_file = "//bazel:spdlog.BUILD",
    urls = ["https://github.com/gabime/spdlog/archive/v1.7.0.tar.gz"]
  )

def com_google_abseil():
  http_archive(
    name = "com_google_absl",
    sha256 = "e3812f256dd7347a33bf9d93a950cf356c61c0596842ff07d8154cd415145d83",
    strip_prefix = "abseil-cpp-5d8fc9192245f0ea67094af57399d7931d6bd53f",
    urls = ["https://github.com/abseil/abseil-cpp/archive/5d8fc9192245f0ea67094af57399d7931d6bd53f.tar.gz"],
  )
