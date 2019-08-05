#pragma once

#include <string>

namespace ringer {
namespace logger {
void init();

// prefix is separated from message with ": "

void info(const std::string& message);
void info(const std::string& prefix, const std::string& message);

void warn(const std::string& message);
void warn(const std::string& prefix, const std::string& message);

void error(const std::string& message);
void error(const std::string& prefix, const std::string& message);

void skipLine(const int totalLine = 1);
}  // namespace logger
}  // namespace ringer