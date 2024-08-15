#ifndef FFEXECUTE_HPP
#define FFEXECUTE_HPP

#include <cstdio>
#include <string>
#include <sstream> // for getInterpretationOfTime
#include <ctime>
#include <fstream>

#include "FFTester.hpp"

typedef std::string str;
typedef const std::string &cstr;

#define FFMPEG_OUTPUT_FILE(time) ("FFmpeg_output-" + time + ".txt") 

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"

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
    static void openFFOFile(); // FFOFile is FFmpegOutputFile
    static void addTextToFFOFile(cstr ffmpegOutput); // FFOFile is FFmpegOutputFile
    static void closeFFOFile(); // FFOFile is FFmpegOutputFile
    static int sizeofDuration(int number);
    static str stringProgress(int progress);
    static void clearLine(int len);

    static void _runFFmpeg(cstr inFile, cstr outFile);
public:
    static str changeExtToMP4(cstr pathToFile);
    static void runFFmpeg(cstr inFile, cstr outFile);

private:
    static int m_performedFFmpegs;
    static int m_correctlyPerformedFFmpegs;
    static std::ofstream m_ffOFile; // ffOFile is FFmpegOutputFile
    static str m_ffOFileName; // ffOFile is FFmpegOutputFile
    static size_t m_duration;
    static int m_sizeofDuration;
};

#endif