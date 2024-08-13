#include "FFExecute.hpp"

size_t FFExecute::m_duration;

void FFExecute::handleOutput(cstr line)
{
    // save to file function...

    if(m_duration == durationNotSet)
    {
        constexpr const char *durationText = "Duration: ";
        constexpr int durationTextSize = 10; // sizeof not work here
        constexpr int strtimeTextSize = 11;
        size_t durationTextPos = line.find(durationText);
        if(durationTextPos == str::npos)
            return;
        
        // example line: "  Duration: 00:00:14.77, start: 0.000000, bitrate: 373 kb/s"
        str strtime = line.substr(durationTextPos + durationTextSize, strtimeTextSize); // gives "00:00:14.77"
        m_duration = FFExecute::getInterpretationOfTime(strtime);
        return;
    }

    // duration is already readed

    constexpr const char *timeText = "kB time="; // kb are for better fit
    constexpr int timeTextSize = 8; // sizeof not work here
    constexpr int strtimeTextSize = 11;
    size_t timeTextPos = line.find("kB time=");
    if(timeTextPos == str::npos)
        return;
        
    str strtime = line.substr(timeTextPos + timeTextSize, strtimeTextSize);
    size_t timePassed = FFExecute::getInterpretationOfTime(strtime);
    printf("time left: %2d%%\n", (timePassed * 100)/m_duration);
    // printf("out: %s\n", line.c_str());
}

size_t FFExecute::getInterpretationOfTime(cstr strtime)
{
    // well coded by chat gpt
    std::stringstream ss(strtime);
    std::string segment;
    size_t hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    std::getline(ss, segment, ':');
    hours = std::stoll(segment);
    
    std::getline(ss, segment, ':');
    minutes = std::stoll(segment);
    
    std::getline(ss, segment, '.');
    seconds = std::stoll(segment);
    std::getline(ss, segment);
    milliseconds = std::stoll(segment);

    return (hours * 3600 * 1000) + 
        (minutes * 60 * 1000) + 
        (seconds * 1000) + 
        milliseconds;
    
}

void FFExecute::runFFmpeg(cstr inFile, cstr outFile)
{
    m_duration = durationNotSet;
    char buffer[128];

    str command = "ffmpeg -i \"" + inFile + "\" -c:v libx265 -vtag hvc1 \"" + outFile + "\"";
    command += "2>&1"; // move stderr to stdout (connect them)

    printf("starting new ffmpeg...\n");

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "Cannot open the pipe!\n");
        return;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        FFExecute::handleOutput(str(buffer));
    }

    pclose(pipe);
    printf("ffmpeg finished!\n");

    return;
}