#include <google/protobuf/util/json_util.h>

#include <string>

#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "cpp2sky/tracer.h"
#include "httplib.h"

int main() {
  using namespace cpp2sky;
  std::string service_name = "e2e";
  std::string instance_name = "consumer";

  auto tracer = createInsecureGrpcTracer("collector:19876");
  Config config(service_name, instance_name, "");
  httplib::Server svr;

  svr.Get("/ping", [&](const httplib::Request &, httplib::Response &res) {
    auto current_segment = createSegmentContext(config);

    auto span = current_segment->createCurrentSegmentSpan(nullptr);
    span->setPeer("");
    span->setStartTime(1);
    span->setOperationName("/ping");
    res.set_content("Hello World!", "text/plain");
    span->setEndTime(2);

    auto object = current_segment->createSegmentObject();
    tracer->sendSegment(object);

    std::string json;
    google::protobuf::util::MessageToJsonString(object, &json);
    std::cout << json << std::endl;
  });

  svr.listen("0.0.0.0", 8081);
  return 0;
}
