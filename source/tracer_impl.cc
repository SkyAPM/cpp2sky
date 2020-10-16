#include "tracer_impl.h"

int TracerImpl::sample() { return 0; }

std::unique_ptr<Tracer> TracerFactoryImpl::create() {
  return std::make_unique<TracerImpl>();
}