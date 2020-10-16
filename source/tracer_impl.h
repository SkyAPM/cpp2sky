#include "sky2cpp/tracer.h"

class TracerImpl : public Tracer {
 public:
  ~TracerImpl() {}

  int sample() override;
};

class TracerFactoryImpl : public TracerFactory {
 public:
  ~TracerFactoryImpl() {}

  std::unique_ptr<Tracer> create() override;
};

std::unique_ptr<TracerFactory> generateTracerFactory() {
  return std::make_unique<TracerFactoryImpl>();
}
