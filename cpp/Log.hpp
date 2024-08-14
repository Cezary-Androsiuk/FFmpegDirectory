#ifndef LOG_HPP
#define LOG_HPP

#include <cstdio>
#include <string>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <sys/time.h>

// convert
#include <vector>

//////////////////// NEW DEBUG ////////////////////
#define I(x) Log::info    (__PRETTY_FUNCTION__, x);
#define W(x) Log::warning (__PRETTY_FUNCTION__, x);
#define E(x) Log::error   (__PRETTY_FUNCTION__, x);
#define D(x) Log::debug   (__PRETTY_FUNCTION__, x);
//////////////////////////////////////////////////


//////////////////// PRINTF DEBUG ////////////////////
#define __p(x, func, ...){\
    size_t s = snprintf(nullptr, 0, x, __VA_ARGS__);\
    char *buffer = new char(s+1);\
    if(buffer == nullptr) {E("Definition __p failed!");}\
    else {\
        snprintf(buffer, s+1, x, __VA_ARGS__);\
        func(__PRETTY_FUNCTION__, buffer);\
    }\
}
#define pI(x, ...) __p( x, Log::info    , __VA_ARGS__ )
#define pW(x, ...) __p( x, Log::warning , __VA_ARGS__ )
#define pE(x, ...) __p( x, Log::error   , __VA_ARGS__ )
#define pD(x, ...) __p( x, Log::debug   , __VA_ARGS__ )
//////////////////////////////////////////////////


#define LOG_FILE "LogHistory.log"
#define MAX_LOG_FILE_SIZE 209'715'200 /// in bytes /// value above what, log file will be trimmed to TRIM_LOG_FILE_SIZE size
#define TRIM_LOG_FILE_SIZE 10'485'760 /// in bytes
#define EST_FUNCTION_LENGTH 70 // estimated function name length what will be reserved while creating log
#define SHORTER_FUNCTION_FILL_CHARACTER ' ' // characters that fills area before function name to fit estimated function name length
#define CONTENT_SPACE 10 // space between function name and content
#define CONTENT_SPACE_CHARACTER ' ' // characters that fills space between function name and content
#define SPACE_BETWEEN_CONTENT_SPACE_AND_CONTENT true // creates spaces between space: "x ........ y" instead of "x........y"

class Log
{
public:
    static void info(std::string func, std::string log);
    static void warning(std::string func, std::string log);
    static void error(std::string func, std::string log);
    static void debug(std::string func, std::string log);

private:
    static std::string time();
    static std::string buildPrefix(std::string time, std::string type, std::string func);

    static void log(std::string content);
    static void print(std::string content);
    static void saveFile(std::string content);

    static void fileSizeProtection();

public:
    class Convert{
    public:
        static std::string vectorStringsToString(std::vector<std::string> list);
    };

    static bool fileSizeProtectionExecuted;
};

#endif // LOG_HPP
