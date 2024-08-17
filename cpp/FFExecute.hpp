#ifndef FFEXECUTE_HPP
#define FFEXECUTE_HPP

#include <cstdio>
#include <string>
#include <sstream> // for getInterpretationOfTime
#include <ctime>
#include <fstream>
#include <filesystem>

#include "FFTester.hpp"

namespace fs = std::filesystem;

typedef std::string str;
typedef const std::string &cstr;

#define FFMPEG_OUTPUT_FILE(time) ("FFmpeg_output-" + time + ".txt") 

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define TEXT_BOLD     "\033[1m"

// #define RESET   "\033[0m"
// #define BLACK   "\033[30m"
// #define RED     "\033[31m"
// #define GREEN   "\033[32m"
// #define YELLOW  "\033[33m"
// #define BLUE    "\033[34m"
// #define MAGENTA "\033[35m"
// #define CYAN    "\033[36m"
// #define WHITE   "\033[37m"
// #define BOLDBLACK   "\033[1m\033[30m"
// #define BOLDRED     "\033[1m\033[31m"
// #define BOLDGREEN   "\033[1m\033[32m"
// #define BOLDYELLOW  "\033[1m\033[33m"
// #define BOLDBLUE    "\033[1m\033[34m"
// #define BOLDMAGENTA "\033[1m\033[35m"
// #define BOLDCYAN    "\033[1m\033[36m"
// #define BOLDWHITE   "\033[1m\033[37m"

// //General Formatting
// #define GEN_FORMAT_RESET                "0"
// #define GEN_FORMAT_BRIGHT               "1"
// #define GEN_FORMAT_DIM                  "2"
// #define GEN_FORMAT_UNDERSCORE           "3"
// #define GEN_FORMAT_BLINK                "4"
// #define GEN_FORMAT_REVERSE              "5"
// #define GEN_FORMAT_HIDDEN               "6"

// //Foreground Colors
// #define FOREGROUND_COL_BLACK            "30"
// #define FOREGROUND_COL_RED              "31"
// #define FOREGROUND_COL_GREEN            "32"
// #define FOREGROUND_COL_YELLOW           "33"
// #define FOREGROUND_COL_BLUE             "34"
// #define FOREGROUND_COL_MAGENTA          "35"
// #define FOREGROUND_COL_CYAN             "36"
// #define FOREGROUND_COL_WHITE            "37"

// //Background Colors
// #define BACKGROUND_COL_BLACK            "40"
// #define BACKGROUND_COL_RED              "41"
// #define BACKGROUND_COL_GREEN            "42"
// #define BACKGROUND_COL_YELLOW           "43"
// #define BACKGROUND_COL_BLUE             "44"
// #define BACKGROUND_COL_MAGENTA          "45"
// #define BACKGROUND_COL_CYAN             "46"
// #define BACKGROUND_COL_WHITE            "47"

// #define SHELL_COLOR_ESCAPE_SEQ(X) "\x1b["X"m"
// #define SHELL_FORMAT_RESET  ANSI_COLOR_ESCAPE_SEQ(GEN_FORMAT_RESET)


// ,,Since a Microsoft compiler is used, an underscore is needed at the beginning''
// https://stackoverflow.com/questions/38876218/execute-a-command-and-get-output-popen-and-pclose-undefined
#define pipeOpen(...) _popen(__VA_ARGS__)
#define pipeClose(...) _pclose(__VA_ARGS__)

class FFExecute
{
    static void handleOutput(cstr line);
    static void printOutputToCMD(cstr line);
    static size_t getInterpretationOfTime(cstr strtime);
    static str getCurrentTime();
    static str changeOutputFileNameIfNeeded(cstr fileName);

    static void openFFOFile(); // FFOFile is FFmpegOutputFile
    static void addTextToFFOFile(cstr ffmpegOutput); // FFOFile is FFmpegOutputFile
    static void closeFFOFile(); // FFOFile is FFmpegOutputFile

    static int lengthOfNumber(int number);

    static void clearLine(int len);
    static void printProgress(int progress);

    static void _runFFmpeg(cstr inFile, str outFile);

public:
    static str makeFileProgressPostfix(bool addColors = true);
    static void setTotalFFmpegsToPerform(int count);
    static void runFFmpeg(cstr inFile, cstr outFile);

private:
    static int m_performedFFmpegs;
    static int m_correctlyPerformedFFmpegs;
    static int m_failedFFmpegs;
    static int m_skippedFFmpegs;
    static int m_totalFFmpegsToPerform;
    
    static std::ofstream m_ffOFile; // ffOFile is FFmpegOutputFile
    static str m_ffOFileName; // ffOFile is FFmpegOutputFile
    static size_t m_duration;
    static int m_lengthOfDuration;
};

#endif