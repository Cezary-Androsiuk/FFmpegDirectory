#include "FFExecute.hpp"

int FFExecute::m_performedFFmpegs = 0;
int FFExecute::m_correctlyPerformedFFmpegs = 0;
std::ofstream FFExecute::m_ffOFile;
str FFExecute::m_ffOFileName;
size_t FFExecute::m_duration;
int FFExecute::m_sizeofDuration;

void FFExecute::handleOutput(cstr line)
{
    FFExecute::addTextToFFOFile(line);
    FFExecute::printOutputToCMD(line);
}

void FFExecute::printOutputToCMD(cstr line)
{
    constexpr int strtimeTextSize = 11;

    constexpr const char timeText[] = "kB time="; // kb are for better match
    size_t timeTextPos = line.find(timeText);
    if(timeTextPos == str::npos)
        return;
        
    str strtime = line.substr(timeTextPos + sizeof(timeText), strtimeTextSize);
    size_t timePassed = FFExecute::getInterpretationOfTime(strtime);

    FFExecute::clearLine(15+2 + m_sizeofDuration * 2);
    str progressValue = str(m_sizeofDuration-FFExecute::sizeofDuration(timePassed), ' ');
    printf("    progress: %s%d/%d", progressValue.c_str(), timePassed, m_duration);
    fflush(stdout);
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
        printf("%s", ffmpegOutput.c_str());
    else
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

void FFExecute::clearLine(int len)
{
    // clear line (windows os only)
    printf("\r"); // move to start
    printf("%s", std::string(len, ' ').c_str());
    printf("\r"); // move to start
}

void FFExecute::_runFFmpeg(cstr inFile, cstr outFile)
{
    // test if inFile extension is valid 
    // test if video is h.265 with ffprobe
    // set m_duration, by using ffprobe
    /// add class variable that stores skipped files, due to file already has h.265, that will show "finished files 10/12 (2 skipped due to h.265)" /// idk what will be better, leaving that file alone, copying (this one seems the best) or moving it to output directory
    
    printf("  Starting new FFmpeg\n");          FFExecute::addTextToFFOFile("  Starting new ffmpeg\n");
    printf("    in:  %s\n", inFile.c_str());    FFExecute::addTextToFFOFile("    in:  " + inFile + "\n");
    printf("    out: %s\n", outFile.c_str());   FFExecute::addTextToFFOFile("    out: " + outFile + "\n");

    if(!FFTester::testIfH265(inFile))
    {
        if(FFTester::errorOccur())
        {
            fprintf(stderr, "    error occur while checking if file is H265: %s\n", 
                FFTester::getErrorInfo().c_str());
            FFExecute::addTextToFFOFile("    error occur while checking if file is H265: " + 
                FFTester::getErrorInfo() + "\n");
            
            return;
        }
        // in file is already H265 format!
        fprintf(stderr, "    inFile is already H265! Skipping!\n");
        FFExecute::addTextToFFOFile("    inFile is already H265! Skipping!\n");

        // possible actins here:
        // 1: skip inFile as it is      <
        // 2: copy inFile to outFile    
        // 3: move inFile to outFile    

        return;
    }

    m_duration = FFExecute::getInterpretationOfTime(FFTester::getStrDuration());
    m_sizeofDuration = FFExecute::sizeofDuration(m_duration);

    str command = "ffmpeg -i \"" + inFile + "\" -c:v libx265 -vtag hvc1 \"" + outFile + "\"";
    command += " 2>&1"; // move stderr to stdout (connect them)
    
    str progressValue = str(m_sizeofDuration-1, ' ');
    printf("    progress: %s0/%d", progressValue.c_str(), m_duration);

    FILE* pipe = pipeOpen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "    Cannot open the pipe!\n");
        FFExecute::addTextToFFOFile("    Cannot open the pipe!\n");
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        FFExecute::handleOutput(str(buffer));

    int ffmpegExitCode = pipeClose(pipe);

    ++ m_performedFFmpegs;


    FFExecute::clearLine(15+2 + m_sizeofDuration * 2);
    if(ffmpegExitCode) // not equal 0 - error occur in ffmpeg
    {
        printf("    progress: %d/%d\n", m_duration, m_duration);
        fprintf(stderr, "    FFmpeg " COLOR_RED "failed" COLOR_RESET " with code %d!\n", ffmpegExitCode);
        FFExecute::addTextToFFOFile("    FFmpeg failed with code " + std::to_string(ffmpegExitCode) + "!\n");
    }
    else // no error - ffmpeg finished correctly
    {
        printf("    progress: %d/%d\n", m_duration, m_duration);
        ++ m_correctlyPerformedFFmpegs;
        fprintf(stderr, "    FFmpeg " COLOR_GREEN "finished" COLOR_RESET "!\n\n");
        FFExecute::addTextToFFOFile("    FFmpeg finished!\n\n");
    }

}

str FFExecute::changeExtToMP4(cstr pathToFile)
{
    size_t dotPos = pathToFile.find_last_of('.');
    return pathToFile.substr(0, dotPos+1) + "mp4";
}

void FFExecute::runFFmpeg(cstr inFile, cstr outFile)
{
    FFExecute::openFFOFile();
    FFExecute::_runFFmpeg(inFile, outFile);
    FFExecute::closeFFOFile();
}