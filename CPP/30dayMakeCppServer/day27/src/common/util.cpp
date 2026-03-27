#include "util.h"
#include <cerrno>
#include <cstring>

#include "Logger.h"

void ErrorIf(bool condition, const char *errmsg) {
  if (condition) {
    LOG_FATAL("%s: %s", errmsg, std::strerror(errno));
  }
}
