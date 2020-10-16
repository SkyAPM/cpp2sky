#include "cpp2sky/tracer.h"

int main(void) {
  auto tracer_factory = generateTracerFactory();
  auto tracer = tracer_factory->create();
  return tracer->sample();
}