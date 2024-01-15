#include <iostream>
#include <string>
#include <vector>

const std::string DEBUG_PREFIX = "[DEBUG]";
const std::string LOG_PREFIX = "[ LOG ]";
const std::string ERROR_PREFIX = "[ERROR]";

class Logger {
private:
    static bool newline;
    static Logger* _instance;
    void _debug(std::string message);
    void _log(std::string message);
    void _error(std::string message);
public:
    static Logger* instance();

    static std::ostream& log();
    static std::ostream& error();
    static std::ostream& debug();

    static int endl();

    static void debug(std::string message);
    static void log(std::string message);
    static void error(std::string message);

    static void logln();
    static void errorln();
    static void debugln();
    
    static void logln(int number);
    
    static void logln(std::string message);
    static void debugln(std::string message);
    static void errorln(std::string message);

    static void log_byte(short byte);

};


template<typename T>
void print(std::ostream& stream, std::vector<T>& vector);