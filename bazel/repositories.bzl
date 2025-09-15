load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cpp2sky_dependencies():
    skywalking_data_collect_protocol()
    com_github_grpc_grpc()
    com_google_googletest()
    com_google_protobuf()
    com_github_httplib()
    com_github_fmtlib_fmt()
    com_google_abseil()
    com_github_gabime_spdlog()
    hedron_compile_commands()

def skywalking_data_collect_protocol():
    http_archive(
        name = "skywalking_data_collect_protocol",
        sha256 = "b7b2df420440b08929280ecb95d0d7fb8ef301f5e3add1f04fef6fb75cf51f61",
        urls = [
            "https://github.com/apache/skywalking-data-collect-protocol/archive/fb3fb005650e2489164978b7804117c7ade1529a.tar.gz",
        ],
        strip_prefix = "skywalking-data-collect-protocol-fb3fb005650e2489164978b7804117c7ade1529a",
    )

def com_github_grpc_grpc():
    http_archive(
        name = "com_github_grpc_grpc",
        sha256 = "7bf97c11cf3808d650a3a025bbf9c5f922c844a590826285067765dfd055d228",
        urls = ["https://github.com/grpc/grpc/archive/refs/tags/v1.74.1.tar.gz"],
        strip_prefix = "grpc-1.74.1",
    )

def com_google_googletest():
    http_archive(
        name = "com_google_googletest",
        sha256 = "65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c",
        strip_prefix = "googletest-1.17.0",
        urls = ["https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz"],
    )

def com_google_protobuf():
    http_archive(
        name = "com_google_protobuf",
        sha256 = "3ad017543e502ffaa9cd1f4bd4fe96cf117ce7175970f191705fa0518aff80cd",
        strip_prefix = "protobuf-32.0",
        urls = ["https://github.com/google/protobuf/archive/refs/tags/v32.0.tar.gz"],
    )

def com_github_httplib():
    http_archive(
        name = "com_github_httplib",
        sha256 = "fcfea48c8f2c386e7085ef8545c8a4875efa30fa6d5cf9dd31f03c6ad038da9d",
        strip_prefix = "cpp-httplib-0.22.0",
        build_file = "//bazel:httplib.BUILD",
        urls = ["https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.22.0.tar.gz"],
    )

def com_github_fmtlib_fmt():
    http_archive(
        name = "com_github_fmtlib_fmt",
        sha256 = "23778bad8edba12d76e4075da06db591f3b0e3c6c04928ced4a7282ca3400e5d",
        strip_prefix = "fmt-8.1.1",
        build_file = "//bazel:fmtlib.BUILD",
        urls = ["https://github.com/fmtlib/fmt/releases/download/8.1.1/fmt-8.1.1.zip"],
    )

def com_github_gabime_spdlog():
    http_archive(
        name = "com_github_gabime_spdlog",
        sha256 = "697f91700237dbae2326b90469be32b876b2b44888302afbc7aceb68bcfe8224",
        strip_prefix = "spdlog-1.10.0",
        build_file = "//bazel:spdlog.BUILD",
        urls = ["https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz"],
    )

def com_google_abseil():
    http_archive(
        name = "com_google_absl",
        sha256 = "9b7a064305e9fd94d124ffa6cc358592eb42b5da588fb4e07d09254aa40086db",
        strip_prefix = "abseil-cpp-20250512.1",
        urls = ["https://github.com/abseil/abseil-cpp/releases/download/20250512.1/abseil-cpp-20250512.1.tar.gz"],
    )

def hedron_compile_commands():
    # Hedron's Compile Commands Extractor for Bazel
    # https://github.com/hedronvision/bazel-compile-commands-extractor
    http_archive(
        name = "hedron_compile_commands",
        url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/dc36e462a2468bd79843fe5176542883b8ce4abe.tar.gz",
        sha256 = "d63c1573eb1daa4580155a1f0445992878f4aa8c34eb165936b69504a8407662",
        strip_prefix = "bazel-compile-commands-extractor-dc36e462a2468bd79843fe5176542883b8ce4abe",
    )
