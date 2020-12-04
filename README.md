# cpp2sky

![cpp2sky test](https://github.com/SkyAPM/cpp2sky/workflows/cpp2sky%20test/badge.svg)

Distributed tracing and monitor SDK in CPP for Apache SkyWalking APM

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

## Basic usage

#### Config

cpp2sky provides simple configuration for tracer. We can set `service name`, `instance name` and `token`.
The detail information is described in [official protobuf definition](https://github.com/apache/skywalking-data-collect-protocol/blob/master/language-agent/Tracing.proto#L57-L67). Also, `token` is only mock so you can't send span to restricted oap.

```cpp
#include <cpp2sky/config.h>

int main() {
  using namespace cpp2sky;

  static const std::string service_name = "service_name";
  static const std::string instance_name = "instance_name";
  static const std::string token = "token";

  Config config(service_name, instance_name, token);
}
```

#### Create tracer

After you constructed config, then setup tracer. Tracer supports gRPC reporter only, also TLS adopted gRPC reporter isn't available now.
TLS adoption and REST tracer will be supported in the future.

```
static const std::string oap_address = "oap:12800";

TracerPtr tracer = createInsecureGrpcTracer(address);
```

#### Fetch propagated span

cpp2sky supports only HTTP tracer now. 
Tracing span will be delivered from `sw8` and `sw8-x` HTTP headers. For more detail, please visit [here](https://github.com/apache/skywalking/blob/08781b41a8255bcceebb3287364c81745a04bec6/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v3.md)
Then, you can create propagated span object by decoding these items.

```
SpanContextPtr parent_span = createSpanContext(parent);
```

#### Create workload segment

Create segment for current workload.

```
SegmentContextPtr current_segment = createSegmentContext(config);
```

#### Create span

First, you must create root span to trace current workload.

```
CurrentSegmentSpanPtr current_span = current_segment->createCurrentSegmentRootSpan();
```

After that, you can create another span to trace another workload, such as RPC to other services.
Note that you must have parent span to create secondary span. It will construct parent-child relation when analysis.

```
CurrentSegmentSpanPtr current_span = current_segment->createCurrentSegmentSpan(current_span);
```

#### Send segment to OAP

Note that SegmentContextPtr is unique pointer. So when you'd like to send data, you must move it and don't refer after sending, 
to avoid undefined behavior.

```
SegmentContextPtr current_segment = createSegmentContext(config);
CurrentSegmentSpanPtr current_span = current_segment->createCurrentSegmentRootSpan();

current_span->setOperationName("sample_workload");
current_span->setStartTime(...);
current_span->setEndTime(...);

tracer->sendSegment(std::move(current_segment));
```
