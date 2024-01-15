#include "logger.hpp"
#include <ostream>

Logger* Logger::_instance = nullptr;
bool Logger::newline = true;

void Logger::_debug(std::string message) {
    std::cout << DEBUG_PREFIX << message << std::endl;
}

void Logger::_log(std::string message) {
    std::cout << LOG_PREFIX <<  message << std::endl;
}

void Logger::_error(std::string message) {
    std::cout << ERROR_PREFIX << message << std::endl;
}

void Logger::debug(std::string message) {
    std::cout << DEBUG_PREFIX << message;
}

std::ostream& Logger::log() {
    if (Logger::newline == false) {
        std::cout << std::endl;
    }
    std::cout << LOG_PREFIX;
    Logger::newline = false;
    return std::cout;
}
std::ostream& Logger::debug() {
    if (Logger::newline == false) {
        std::cout << std::endl;
    }
    std::cout << DEBUG_PREFIX;
    Logger::newline = false;
    return std::cout;
}

std::ostream& Logger::error() {
    if (Logger::newline == false) {
        std::cout << std::endl;
    }
    std::cout << ERROR_PREFIX;
    Logger::newline = false;
    return std::cout;
}

void Logger::logln() {
    if (Logger::newline == false) {
        std::cout << std::endl;
        Logger::newline = true;
    }
}

void Logger::errorln() {
    if (Logger::newline == false) {
        std::cout << std::endl;
        Logger::newline = true;
    }
}

void Logger::debugln() {
    if (Logger::newline == false) {
        std::cout << std::endl;
        Logger::newline = true;
    }
}

void Logger::log(std::string message) {
    std::cout << LOG_PREFIX <<  message;
}

void Logger::error(std::string message) {
    std::cout << ERROR_PREFIX << message;
}

void Logger::debugln(std::string message) {
    std::cout << DEBUG_PREFIX << message << std::endl;
}

void Logger::logln(int message) {
    std::cout << LOG_PREFIX <<  message << std::endl;
}

void Logger::logln(std::string message) {
    std::cout << LOG_PREFIX <<  message << std::endl;
}

void Logger::errorln(std::string message) {
    std::cout << ERROR_PREFIX << message << std::endl;
}

void Logger::log_byte(short byte) {
    int value = byte;
    std::cout << std::to_string(value) + " ";
}

Logger* Logger::instance() {
    if (Logger::_instance == nullptr) {
        _instance = new Logger();
    }
    return _instance;
}

template<typename T>
void print(std::ostream& stream, std::vector<T>& vector) {
    for(auto it = vector.begin(); it != vector.end(); ++it) {
        stream << std::to_string(*it) << " ";
    };
}

template void print<int>(std::ostream& stream, std::vector<int>& vector);