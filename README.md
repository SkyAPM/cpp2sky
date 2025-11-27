# cpp2sky

![cpp2sky test](https://github.com/SkyAPM/cpp2sky/workflows/cpp2sky%20test/badge.svg)

Distributed tracing and monitor SDK in CPP for Apache SkyWalking APM. This SDK is compatible with C++ 17, C++ 14, and C++ 11.

## Build

#### Bazel

Download cpp2sky tarball with specified version.

```
http_archive(
  name = "com_github_skyapm_cpp2sky",
  sha256 = <SHA256>,
  urls = ["https://github.com/skyAPM/cpp2sky/archive/<VERSION>.tar.gz"],
)
```

Add interface definition and library to your project.

```
cc_binary(
  name = "example",
  srcs = ["example.cc"],
  deps = [
    "@com_github_skyapm_cpp2sky//cpp2sky:cpp2sky_interface",
    "@com_github_skyapm_cpp2sky//source:cpp2sky_lib"
  ],
)
```

#### Cmake

You can compile this project in one of three supported ways (submodule-first is recommended):

- Recommended — Submodule-first (most reproducible):

```bash
git clone --recurse-submodules git@github.com:SkyAPM/cpp2sky.git
git submodule update --init --recursive
```

This repository pins several third-party dependencies under `3rdparty/` (example: `spdlog`, `fmt`, `httplib`, `skywalking-data-collect-protocol`). The top-level CMake will auto-detect these submodules and use them via `add_subdirectory()`.

 - FetchContent fallback (automatic clone at configure time):

If a submodule is not present, CMake can automatically download the dependency at configure time using FetchContent. This is controlled by CMake options of the form `-D<LIB>_FETCHCONTENT=ON`.

Important: this project now defaults `*_FETCHCONTENT` to `OFF` to favour a submodule-first workflow (reproducible builds). Each dependency module will auto-detect a local `3rdparty/<lib>` submodule and, unless you explicitly set the corresponding `-D` option, enable the submodule and only enable FetchContent as a fallback when the submodule is absent.

FetchContent is declared uniformly using `GIT_REPOSITORY` + `GIT_TAG` where possible so the build can be pinned to a tag or an exact commit SHA. We strongly encourage this pattern because it makes bumps and temporary testing against a branch/commit straightforward.

Recommended FetchContent pattern (preferred)

```cmake
# variables make bumps easy and visible in the cmake file
set(FMTLIB_GIT_URL https://github.com/fmtlib/fmt.git)
set(FMTLIB_GIT_TAG  8.1.1)           # or a commit SHA like `d6a5b8f...`

FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY ${FMTLIB_GIT_URL}
  GIT_TAG        ${FMTLIB_GIT_TAG}
)
FetchContent_MakeAvailable(fmtlib)
```

Why this is useful
- `GIT_TAG` accepts tags, branches or a full commit SHA — use a SHA to pin an exact commit that isn't tagged.
- Using named variables (`*_GIT_URL`, `*_GIT_TAG`) makes automated bump scripts and review diffs clearer.

Notes on projects with nested submodules
- Some repositories (notably gRPC) include their own git submodules. For those projects we recommend either:
  - Use the release archive (`URL` + `URL_HASH`) which avoids nested submodule handling, or
  - Use `GIT_REPOSITORY` + `GIT_TAG` but initialize nested submodules after `FetchContent_Populate`:

```cmake
FetchContent_GetProperties(grpc)
if(NOT grpc_POPULATED)
  FetchContent_Populate(grpc)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                  WORKING_DIRECTORY ${grpc_SOURCE_DIR})
  add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR})
endif()
```

In this repository we prefer the archive approach for gRPC in CI for simplicity, but the git+tag approach is supported and useful for local testing or when you need to pin to a commit SHA.

If you need to force FetchContent for any dependency (for example, to debug or when you prefer not to initialize submodules), you can pass `-D<LIB>_FETCHCONTENT=ON` on the `cmake` command line. If you pass conflicting options (both `-D<LIB>_AS_SUBMODULE=ON` and `-D<LIB>_FETCHCONTENT=ON`) CMake will stop with a helpful error and you must choose one.

For SkyWalking you can enable FetchContent with:

```bash
cmake -DSKYWALKING_FETCHCONTENT=ON -S . -B build
cmake --build build
```

- Explicit path (developer workflow):

If you already have a local checkout of `skywalking-data-collect-protocol`, point CMake to it:

```bash
cmake -DSKYWALKING_PROTOCOL_PATH=/path/to/skywalking-data-collect-protocol -S . -B build
cmake --build build
```

Notes about dependencies with special handling

Most third-party dependencies follow the same pattern: prefer the pinned submodule under `3rdparty/` (submodule-first) and fall back to `FetchContent` at configure time when the submodule is not present. See the top-level `CMakeLists.txt` and the modules in `cmake/` for details.

There are two cases worth calling out because their consumption/build steps differ slightly:

- gRPC

  gRPC is not header-only and typically needs configuration and a build step (or its targets must be available via `find_package`). This repository includes a pinned copy of gRPC at `3rdparty/grpc` (tag `v1.74.1`) so builds are reproducible. CI is configured to build gRPC from that submodule.

  To build gRPC locally from the submodule and install it for the rest of the project:

  ```bash
  # initialize submodules (if you haven't already)
  git submodule update --init --recursive

  # install build deps
  sudo apt-get update
  sudo apt-get install -y cmake build-essential

  # configure & install gRPC from the submodule
  cmake -S 3rdparty/grpc -B 3rdparty/grpc/build
  cmake --build 3rdparty/grpc/build --parallel 8 --target install

  # configure & build cpp2sky
  cmake -S . -B build
  cmake --build build --parallel $(nproc)
  ```

  If you prefer not to use the submodule you can still clone and build gRPC separately and make its CMake targets available to the project, but using the pinned submodule is recommended for reproducibility.

- skywalking-data-collect-protocol (protobufs)

  The SkyWalking protocol repository contains the protobuf definitions used to generate code for the project. The `cmake/skywalking.cmake` module sets `SKYWALKING_PROTOCOL_PATH` when the `3rdparty/skywalking-data-collect-protocol` submodule is present so the proto generation step can locate the `.proto` files.

  You can override that behavior by supplying an explicit path:

  ```bash
  cmake -DSKYWALKING_PROTOCOL_PATH=/path/to/skywalking-data-collect-protocol -S . -B build
  cmake --build build
  ```

  Alternatively, enable FetchContent for SkyWalking with `-DSKYWALKING_FETCHCONTENT=ON` to let CMake fetch the proto repo at configure time.

How to bump a submodule (example for skywalking-data-collect-protocol):
```bash
cd 3rdparty/skywalking-data-collect-protocol
git fetch --tags
git checkout tags/v10.4.0    # or a specific commit
cd ../..
git add 3rdparty/skywalking-data-collect-protocol
git commit -m "Pin skywalking-data-collect-protocol to v10.4.0"
git push
```

If you prefer CI to always fetch the latest submodules, ensure the workflow initializes submodules (this repo's CI uses `actions/checkout` with `submodules: 'recursive'`).

You can also use find_package to get target libary in your project. Like this:
```
find_package(cpp2sky CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} cpp2sky::cpp2sky proto_lib)
```
Of course, if OS is similar to Unix, you can also use pkgconfig to build the project. Like this:
```
find_package(PkgConfig REQUIRED)
pkg_check_modules(CPP2SKY_PKG REQUIRED cpp2sky)
```

Note:
- If you want to build this project over c11, you must update grpc version(current version:v1.46.6).
- Only test cmake using Centos and Ubuntu.

#### Develop

Generate `compile_commands.json` for this repo by `bazel run :refresh_compile_commands`. Thank https://github.com/hedronvision/bazel-compile-commands-extractor for it provide the great script/tool to make this so easy!

#### Docs

cpp2sky configration is based on protobuf, and docs are generated by [protodoc](https://github.com/etcd-io/protodoc). If you have any API change, you should run below.

```
protodoc --directory=./cpp2sky --parse="message" --languages="C++" --title=cpp2sky config --output=docs/README.md
```

## Basic usage

#### Config

cpp2sky provides simple configuration for tracer. API docs are available at `docs/README.md`.
The detail information is described in [official protobuf definition](https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto#L57-L67).

```cpp
#include <cpp2sky/config.pb.h>

int main() {
  using namespace cpp2sky;

  static const std::string service_name = "service_name";
  static const std::string instance_name = "instance_name";
  static const std::string oap_addr = "oap:12800";
  static const std::string token = "token";

  TracerConfig tracer_config;

  config.set_instance_name(instance_name);
  config.set_service_name(service_name);
  config.set_address(oap_addr);
  config.set_token(token);
}
```

#### Create tracer

After you constructed config, then setup tracer. Tracer supports gRPC reporter only, also TLS adopted gRPC reporter isn't available now.
TLS adoption and REST tracer will be supported in the future.

```cpp
TracerConfig tracer_config;

// Setup

TracerPtr tracer = createInsecureGrpcTracer(tracer_config);
```

#### Fetch propagated span

cpp2sky supports only HTTP tracer now.
Tracing span will be delivered from `sw8` and `sw8-x` HTTP headers. For more detail, please visit [here](https://github.com/apache/skywalking/blob/08781b41a8255bcceebb3287364c81745a04bec6/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v3.md)
Then, you can create propagated span object by decoding these items.

```cpp
SpanContextSharedPtr parent_span = createSpanContext(parent);
```

#### Create span

First, you must create tracing context that holds all spans, then crete initial entry span.

```cpp
TracingContextSharedPtr tracing_context = tracer->newContext();
TracingSpanSharedPtr tracing_span = tracing_context->createEntrySpan();
```

After that, you can create another span to trace another workload, such as RPC to other services.
Note that you must have parent span to create secondary span. It will construct parent-child relation when analysis.

```cpp
TracingSpanSharedPtr current_span = tracing_context->createExitSpan(current_span);
```

Alternative approach is RAII based one. It is used like below,

```cpp
{
  StartEntrySpan entry_span(tracing_context, "sample_op1");
  {
    StartExitSpan exit_span(tracing_context, entry_span.get(), "sample_op2");

    // something...
  }
}
```

#### Send segment to OAP

Note that TracingContext is unique pointer. So when you'd like to send data, you must move it and don't refer after sending,
to avoid undefined behavior.

```cpp
TracingContextSharedPtr tracing_context = tracer->newContext();
TracingSpanSharedPtr tracing_span = tracing_context->createEntrySpan();

tracing_span->startSpan("sample_workload");
tracing_span->endSpan();

tracer->report(std::move(tracing_context));
```

#### Skywalking CDS

C++ agent implements Skywalking CDS feature it allows to change bootstrap config dynamically from the response of sync request, invoked from this periodically.
Dynamically configurable values are described in description of properties on `docs/README.md'.

```cpp
TracerConfig config;
// If you set this value as zero, CDS request won't occur.
config.set_cds_request_interval(5); // CDS request interval should be 5sec
```

If you are using Consul KVS as backend, we could put configuration value through HTTP request.

```yaml
configurations:
  service_name:
    ignore_suffix: '/ignore, /hoge'
```

After setup configurations, try to put values with

```
curl --request PUT --data-binary "@./config.yaml" http://localhost:8500/v1/kv/configuration-discovery.default.agentConfigurations
```

## Trace and Log integration

cpp2sky implements to output logs which is the key to integrate with actual tracing context.

#### Supported Logger

- [spdlog](https://github.com/gabime/spdlog)

```cpp
#include <spdlog/spdlog.h>
#include <cpp2sky/trace_log.h>

int main() {
  auto logger = spdlog::default_logger();
  // set_pattern must be called.
  logger->set_pattern(logFormat<decltype(logger)::element_type>());

  // It will generate log message as follows.
  //
  // {"level": "warning", "msg": "sample", "SW_CTX": ["service","instance","trace_id","segment_id","span_id"]}
  //
  logger->warn(tracing_context->logMessage("sample"));
}
```

## Security

If you've found any security issues, please read [Security Reporting Process](https://github.com/SkyAPM/cpp2sky/blob/main/SECURITY.md) and take described steps.

## LICENSE

Apache 2.0 License. See [LICENSE](https://github.com/SkyAPM/cpp2sky/blob/main/LICENSE) for more detail.
