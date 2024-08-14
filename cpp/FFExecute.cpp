#include "FFExecute.hpp"

int FFExecute::m_performedFFmpegs = 0;
int FFExecute::m_correctlyPerformedFFmpegs = 0;
std::ofstream FFExecute::m_ffOFile;
str FFExecute::m_ffOFileName;
size_t FFExecute::m_duration;
size_t FFExecute::m_lastProgress;

void FFExecute::handleOutput(cstr line)
{
    FFExecute::addTextToFFOFile(line);
    FFExecute::printOutputToCMD(line);
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
    printf("  time left: %2d%%\n", (timePassed * 100)/m_duration);
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

str FFExecute::getCurrentTime()
{
    time_t rawTime;
    time(&rawTime);
    tm *time = localtime(&rawTime);

    char buffer[16];
    sprintf(buffer, "%04d%02d%02d_%02d%02d%02d", 
        time->tm_year, time->tm_mon, time->tm_mday,
        time->tm_hour, time->tm_min, time->tm_sec);
    return str(buffer);
}

void FFExecute::openFFOFile()
{
    if(m_ffOFileName.empty())
        m_ffOFileName = FFMPEG_OUTPUT_FILE(FFExecute::getCurrentTime());

    m_ffOFile = std::ofstream(m_ffOFileName, std::ios::app);
    if(!m_ffOFile.good())
    {
        // i won't fuck with that here...
        // will be handled in addTextToFFOFile
        fprintf(stderr, "Error while oppening ffmpeg output file!");
    }
}

void FFExecute::addTextToFFOFile(cstr ffmpegOutput)
{
    if(!m_ffOFile.good())
    {
        printf("%s", ffmpegOutput.c_str());
    }
    m_ffOFile << ffmpegOutput;
}

void FFExecute::closeFFOFile()
{
    m_ffOFile << "\n";
    m_ffOFile.close();
}

void FFExecute::runFFmpeg(cstr inFile, cstr outFile)
{
    m_duration = durationNotSet;
    FFExecute::openFFOFile();

    str command = "ffmpeg -i \"" + inFile + "\" -c:v libx265 -vtag hvc1 \"" + outFile + "\"";
    command += " 2>&1"; // move stderr to stdout (connect them)
    
    printf("Starting new ffmpeg\n");          FFExecute::addTextToFFOFile("Starting new ffmpeg\n");
    printf("  in:  %s\n", inFile.c_str());    FFExecute::addTextToFFOFile("  in:  " + inFile + "\n");
    printf("  out: %s\n", outFile.c_str());   FFExecute::addTextToFFOFile("  out: " + outFile + "\n");

    FILE* pipe = pipeOpen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "Cannot open the pipe!\n");
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        FFExecute::handleOutput(str(buffer));

    int exitCode = pipeClose(pipe);

    ++ m_performedFFmpegs;

    if(exitCode) // not equal 0 - error occur in ffmpeg
    {

    }
    else // no error - ffmpeg finished correctly
    {
        printf("");
        ++ m_correctlyPerformedFFmpegs;
    }

    printf("  ffmpeg finished with status: %d\n", exitCode);

    FFExecute::closeFFOFile();

    return;
}