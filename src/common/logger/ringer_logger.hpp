#pragma once

#include <string>

namespace ringer {
namespace logger {
void init(const std::string& prefix);

void info(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);

void skipLine(const int totalLine = 1);
}  // namespace logger
}  // namespace ringer