#include "httplib.h"

int main() {
  httplib::Server svr;
  svr.Get("/ping", [](const httplib::Request&, httplib::Response &res) {
    res.set_content("Hello World!", "text/plain");
  });
  svr.listen("0.0.0.0", 8081);
  return 0;
}
