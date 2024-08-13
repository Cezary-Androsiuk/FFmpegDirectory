#ifndef FFEXECUTE_HPP
#define FFEXECUTE_HPP

#include <cstdio>
#include <string>
#include <sstream> // for getInterpretationOfTime

typedef std::string str;
typedef const std::string &cstr;

constexpr size_t durationNotSet = -1;

class FFExecute{

    static void handleOutput(cstr line);
    static size_t getInterpretationOfTime(cstr strtime);
public:
    static void runFFmpeg(cstr inFile, cstr outFile);

private:
    static size_t m_duration;
};

#endif