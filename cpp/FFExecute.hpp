#ifndef FFEXECUTE_HPP
#define FFEXECUTE_HPP

#include <cstdio>
#include <string>
#include <sstream> // for getInterpretationOfTime
#include <ctime>
#include <fstream>

typedef std::string str;
typedef const std::string &cstr;

#define FFMPEG_OUTPUT_FILE(time) ("FFmpeg_output-" + time + ".txt") 

constexpr size_t durationNotSet = -1;

// ,,Since a Microsoft compiler is used, an underscore is needed at the beginning''
// https://stackoverflow.com/questions/38876218/execute-a-command-and-get-output-popen-and-pclose-undefined
#define pipeOpen(...) _popen(__VA_ARGS__)
#define pipeClose(...) _pclose(__VA_ARGS__)

class FFExecute{
    static void handleOutput(cstr line);
    static void printOutputToCMD(cstr line);
    static size_t getInterpretationOfTime(cstr strtime);
    static str getCurrentTime();
    static void openFFOFile(); // FFOFile is FFmpegOutputFile
    static void addTextToFFOFile(cstr ffmpegOutput); // FFOFile is FFmpegOutputFile
    static void closeFFOFile(); // FFOFile is FFmpegOutputFile

public:
    static void runFFmpeg(cstr inFile, cstr outFile);

private:
    static int m_performedFFmpegs;
    static int m_correctlyPerformedFFmpegs;
    static std::ofstream m_ffOFile;
    static str m_ffOFileName; // ffOFile is FFmpegOutputFile
    static size_t m_duration;
    static size_t m_lastProgress;
};

#endif