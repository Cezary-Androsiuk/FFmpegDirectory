#include "FFExecute.hpp"

size_t FFExecute::m_duration;
size_t FFExecute::m_lastProgress;

void FFExecute::handleOutput(cstr line)
{
    // printf("%s\n", line.c_str());
    // save to file function...

    FFExecute::printOutputToCMD(line);

    // printf("out: %s\n", line.c_str());
}
void FFExecute::saveOutputToFile(cstr line)
{

}
void FFExecute::printOutputToCMD(cstr line)
{
    if(m_duration == durationNotSet)
    {
        constexpr const char durationText[] = "Duration: ";
        // constexpr int durationTextSize = 10;
        constexpr int strtimeTextSize = 11;
        size_t durationTextPos = line.find(durationText);
        if(durationTextPos == str::npos)
            return;
        
        // example line: "  Duration: 00:00:14.77, start: 0.000000, bitrate: 373 kb/s"
        str strtime = line.substr(durationTextPos + sizeof(durationText), strtimeTextSize); // gives "00:00:14.77"
        m_duration = FFExecute::getInterpretationOfTime(strtime);
        m_lastProgress = 0;
        return;
    }

    // duration is already readed

    constexpr const char timeText[] = "kB time="; // kb are for better match
    // constexpr int timeTextSize = 8; // sizeof not work here
    constexpr int strtimeTextSize = 11;
    size_t timeTextPos = line.find("kB time=");
    if(timeTextPos == str::npos)
        return;
        
    str strtime = line.substr(timeTextPos + sizeof(timeText), strtimeTextSize);
    size_t timePassed = FFExecute::getInterpretationOfTime(strtime);
    printf("time left: %2d%%\n", (timePassed * 100)/m_duration);
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
    command += " 2>&1"; // move stderr to stdout (connect them)
    I("executing command: " + command);

    I("starting new ffmpeg...");

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "Cannot open the pipe!\n");
        return;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        FFExecute::handleOutput(str(buffer));
    }

    int returnCode = pclose(pipe);
    printf("ffmpeg finished with status: %d\n", returnCode);

    return;
}