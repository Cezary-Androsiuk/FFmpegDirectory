#include "FFExecute.hpp"

int FFExecute::m_performedFFmpegs = 0;
int FFExecute::m_correctlyPerformedFFmpegs = 0;
std::ofstream FFExecute::m_ffOFile;
str FFExecute::m_ffOFileName;
size_t FFExecute::m_duration;
int FFExecute::m_sizeofDuration;
size_t FFExecute::m_lastProgress;

void FFExecute::handleOutput(cstr line)
{
    FFExecute::addTextToFFOFile(line);
    FFExecute::printOutputToCMD(line);
}

void FFExecute::printOutputToCMD(cstr line)
{
    constexpr int strtimeTextSize = 11;

    if(m_duration == durationNotSet)
    {
        constexpr const char durationText[] = "Duration: ";
        size_t durationTextPos = line.find(durationText);
        if(durationTextPos == str::npos)
            return;
        
        // example line: "  Duration: 00:00:14.77, start: 0.000000, bitrate: 373 kb/s"
        str strtime = line.substr(durationTextPos + sizeof(durationText), strtimeTextSize); // gives "00:00:14.77"
        m_duration = FFExecute::getInterpretationOfTime(strtime);
        m_lastProgress = 0;
        return;
    }

    // clear line (windows os only)
    printf("\r"); // move to start
    printf("%s", std::string(17 + m_sizeofDuration * 2, ' ').c_str());
    printf("\r"); // move to start

    constexpr const char timeText[] = "kB time="; // kb are for better match
    size_t timeTextPos = line.find(timeText);
    if(timeTextPos == str::npos)
        return;
        
    str strtime = line.substr(timeTextPos + sizeof(timeText), strtimeTextSize);
    size_t timePassed = FFExecute::getInterpretationOfTime(strtime);

    printf("    progress:  %s", stringProgress(timePassed).c_str());
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

int FFExecute::sizeofDuration(int number)
{
    return std::to_string(number).size();
}

str FFExecute::stringProgress(int progress)
{
    // create format __23/0123, or _123/0123
    str progressStr = std::to_string(progress);
    int sizeofProgress = progressStr.size();
    progressStr = std::string(m_sizeofDuration - sizeofProgress, ' ') + progressStr;
    return progressStr + "/" + std::to_string(m_duration);
}

str FFExecute::changeExtToMP4(cstr pathToFile)
{
    size_t dotPos = pathToFile.find_last_of('.');
    return pathToFile.substr(0, dotPos+1) + "mp4";
}

void FFExecute::runFFmpeg(cstr inFile, cstr outFile)
{
    // test if inFile extension is valid 
    // test if video is h.265 with ffprobe
    // set m_duration, by using ffprobe

    m_duration = durationNotSet;
    m_sizeofDuration = FFExecute::sizeofDuration(m_duration);
    FFExecute::openFFOFile();

    str command = "ffmpeg -i \"" + inFile + "\" -c:v libx265 -vtag hvc1 \"" + outFile + "\"";
    command += " 2>&1"; // move stderr to stdout (connect them)
    
    printf("  Starting new FFmpeg\n");          FFExecute::addTextToFFOFile("  Starting new ffmpeg\n");
    printf("    in:  %s\n", inFile.c_str());    FFExecute::addTextToFFOFile("    in:  " + inFile + "\n");
    printf("    out: %s\n", outFile.c_str());   FFExecute::addTextToFFOFile("    out: " + outFile + "\n");

    printf("    progress:  %s", stringProgress(0).c_str()); // start display info /// / (duration size but 0)/m_duration 

    FILE* pipe = pipeOpen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "Cannot open the pipe!\n");
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        FFExecute::handleOutput(str(buffer));

    int ffmpegExitCode = pipeClose(pipe);

    ++ m_performedFFmpegs;

    printf("\n"); // handle output, does not adds that

    if(ffmpegExitCode) // not equal 0 - error occur in ffmpeg
    {
        printf("    FFmpeg " COLOR_RED "failed" COLOR_RESET " with code %d!\n", ffmpegExitCode);
        FFExecute::addTextToFFOFile("    FFmpeg failed with code " + std::to_string(ffmpegExitCode) + "!\n");
    }
    else // no error - ffmpeg finished correctly
    {
        ++ m_correctlyPerformedFFmpegs;
        printf("    FFmpeg " COLOR_GREEN "finished" COLOR_RESET "!\n\n");
        FFExecute::addTextToFFOFile("    FFmpeg finished!\n\n");
    }

    FFExecute::closeFFOFile();
}