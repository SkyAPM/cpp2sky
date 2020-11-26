#include <string>

#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "cpp2sky/tracer.h"
#include "httplib.h"

int main() {
  using namespace cpp2sky;
  std::string service_name = "e2e";
  std::string instance_name = "consumer";

  auto tracer = createInsecureGrpcTracer("localhost:19876");
  Config config(service_name, instance_name, "");
  httplib::Server svr;

  svr.Get("/ping", [&](const httplib::Request &, httplib::Response &res) {
    auto span_ctx = createSpanContext("");
    auto current_segment = createSegmentContext(config, span_ctx);
    res.set_content("Hello World!", "text/plain");
    auto m = current_segment->createSegmentObject();
    tracer->sendSegment(m);
  });
  svr.listen("0.0.0.0", 8080);
  return 0;
}
