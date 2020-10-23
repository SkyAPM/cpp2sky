#include <memory>

class Tracer {
 public:
  virtual ~Tracer() = default;

  virtual int sample() = 0;
};

class TracerFactory {
 public:
  virtual ~TracerFactory() = default;

  virtual std::unique_ptr<Tracer> create() = 0;
};

std::unique_ptr<TracerFactory> generateTracerFactory();
