#include "ringer_logger.hpp"

#include <algorithm>
#include <iostream>

namespace ringer {
namespace logger {
namespace {
// formatting code
static const std::string kBold      = "\033[1m";
static const std::string kUnderline = "\033[4m";
static const std::string kItalic    = "\033[3m";
static const std::string kFaint     = "\033[2m";
static const std::string kReset     = "\033[0m";

// color code
static const std::string kBlue      = "\033[38;5;33m";
static const std::string kLightBlue = "\033[38;5;33m";

static const std::string kDefaultGreen  = "\033[38;5;2m";
static const std::string kMidGreen      = "\033[38;5;084m";
static const std::string kLightMidGreen = "\033[38;5;156m";

static const std::string kLightGreen = "\033[38;5;106m";
static const std::string kGreen      = "\033[38;5;154m";
static const std::string kDarkGreen  = "\033[38;5;34m";

static const std::string kCyan      = "\033[38;5;051m";
static const std::string kLightCyan = "\033[38;5;123m";

static const std::string kYellow     = "\033[38;5;226m";
static const std::string kDarkYellow = "\033[38;5;220m";

static const std::string kDefaultRed = "\033[38;5;9m";

static const std::string kGrey = "\033[38;5;246m";

static std::string logPrefix;

const std::string spacing = "\t  ";
std::string       formatSquareBracket(const std::string& content, const std::string& fmt) {
    return fmt + "[" + content + "]" + kReset + spacing;
}
std::string formatMessage(const std::string& message) { return message + "\n"; }

void logHelper(const std::string& logTypeName,
               const std::string& logFmt,
               const std::string& message) {
    std::cout << formatSquareBracket(logTypeName, logFmt) +
                     formatSquareBracket(logPrefix, kYellow) + formatMessage(message)
              << std::flush;
}
}  // namespace

void init(const std::string& prefix) {
    logPrefix = prefix;
    info("log initialized");
}

void info(const std::string& message) { logHelper("info", kDefaultGreen, message); }

void warn(const std::string& message) { logHelper("warn", kDarkYellow, message); }

void error(const std::string& message) { logHelper("error", kDefaultRed, message); }

void skipLine(const int totalLine) {
    const std::string lineStr(static_cast<size_t>(totalLine), '\n');
    std::cout << lineStr << std::flush;
}
}  // namespace logger
}  // namespace ringer