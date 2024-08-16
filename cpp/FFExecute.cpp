#include "FFExecute.hpp"

int FFExecute::m_performedFFmpegs = 0;
int FFExecute::m_correctlyPerformedFFmpegs = 0;
int FFExecute::m_failedFFmpegs = 0;
int FFExecute::m_skippedFFmpegs = 0;
int FFExecute::m_totalFFmpegsToPerform = 0;

std::ofstream FFExecute::m_ffOFile;
str FFExecute::m_ffOFileName;
size_t FFExecute::m_duration;
int FFExecute::m_lengthOfDuration = 0;

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
    int timePassed = FFExecute::getInterpretationOfTime(strtime);

    FFExecute::printProgress(timePassed);
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
        printf("ffmpeg output file failed, output text: %s", ffmpegOutput.c_str());
    else
        m_ffOFile << ffmpegOutput;
}

void FFExecute::closeFFOFile()
{
    m_ffOFile << "\n";
    m_ffOFile.close();
}

int FFExecute::lengthOfNumber(int number)
{
    return std::to_string(number).size();
}

void FFExecute::clearLine(int len)
{
    // clear line (windows os only)
    printf("\r"); // move to start
    printf("%s", std::string(len, ' ').c_str());
    printf("\r"); // move to start
}

void FFExecute::printProgress(int progress)
{
    // create format __23/0123, or _123/0123
    FFExecute::clearLine(15+2 + m_lengthOfDuration * 2);
    str space = str(m_lengthOfDuration - FFExecute::lengthOfNumber(progress), ' ');
    printf("    progress: %s%d/%d", space.c_str(), progress, m_duration);
    fflush(stdout);
}

str FFExecute::makeFileProgressPostfix()
{
    // total_ffmpegs_to_perform should be the largest number
    int lengthOfCount = FFExecute::lengthOfNumber(m_totalFFmpegsToPerform);
    str cpFFSpace = str(lengthOfCount - FFExecute::lengthOfNumber(m_correctlyPerformedFFmpegs), ' ');
    str pFFSpace = str(lengthOfCount - FFExecute::lengthOfNumber(m_performedFFmpegs), ' ');
    str tFFtpSpace = str(lengthOfCount - FFExecute::lengthOfNumber(m_totalFFmpegsToPerform), ' ');
    str fFFSpace = str(lengthOfCount - FFExecute::lengthOfNumber(m_failedFFmpegs), ' ');
    str sFFSpace = str(lengthOfCount - FFExecute::lengthOfNumber(m_skippedFFmpegs), ' ');
    str cpFF = cpFFSpace + std::to_string(m_correctlyPerformedFFmpegs);
    str pFF = pFFSpace +  std::to_string(m_performedFFmpegs);
    str tFFtp = tFFtpSpace +  std::to_string(m_totalFFmpegsToPerform);
    str fFF = fFFSpace +  std::to_string(m_failedFFmpegs);
    str sFF = sFFSpace +  std::to_string(m_skippedFFmpegs);

    // correctly_performed_ffmpegs / performed_ffmpegs / total_ffmpegs_to_perform   failed_ffmpegs / skipped_ffmpegs
    return cpFF + "/" + pFF + "/" + tFFtp + " " + fFF + "/" + sFF;
}

void FFExecute::_runFFmpeg(cstr inFile, cstr outFile)
{
    // correctly_performed_ffmpegs / performed_ffmpegs / total_ffmpegs_to_perform   failed_ffmpegs / skipped_ffmpegs
    str filesProgress = FFExecute::makeFileProgressPostfix();

    printf("  Starting new FFmpeg [ " COLOR_WHITE/*grey color*/ "%s" COLOR_RESET " ]\n", filesProgress.c_str());
    FFExecute::addTextToFFOFile("  Starting new ffmpeg [ " + filesProgress + " ]\n");

    printf("    in:  %s\n", inFile.c_str());    FFExecute::addTextToFFOFile("    in:  " + inFile + "\n");
    printf("    out: %s\n", outFile.c_str());   FFExecute::addTextToFFOFile("    out: " + outFile + "\n");

    if(FFTester::testIfH265(inFile))
    {
        if(FFTester::errorOccur())
        {
            fprintf(stderr, "    error occur while checking if file is H265: %s\n\n", 
                FFTester::getErrorInfo().c_str());
            FFExecute::addTextToFFOFile("    error occur while checking if file is H265: " + 
                FFTester::getErrorInfo() + "\n\n");
            
            ++ m_failedFFmpegs;
            return;
        }
        // in file is already H265 format!
        fprintf(stderr, "    inFile is already H265! Skipping!\n\n");
        FFExecute::addTextToFFOFile("    inFile is already H265! Skipping!\n\n");

        // possible actins here:
        // 1: skip inFile as it is      <
        // 2: copy inFile to outFile    
        // 3: move inFile to outFile    

        ++ m_skippedFFmpegs;
        return;
    }

    str command = "ffmpeg -i \"" + inFile + "\" -c:v libx265 -vtag hvc1 \"" + outFile + "\"";
    command += " 2>&1"; // move stderr to stdout (connect them)

    m_duration = FFExecute::getInterpretationOfTime(FFTester::getStrDuration());
    m_lengthOfDuration = FFExecute::lengthOfNumber(m_duration);
    FFExecute::printProgress(0);


    
    FILE* pipe = pipeOpen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "    " COLOR_RED "Cannot open the pipe" COLOR_RESET "!\n\n");
        FFExecute::addTextToFFOFile("    Cannot open the pipe!\n\n");
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        FFExecute::handleOutput(str(buffer));

    int ffmpegExitCode = pipeClose(pipe);


    
    if(ffmpegExitCode) // not equal 0 - error occur in ffmpeg
    {
        ++ m_failedFFmpegs;
        printf("\n");
        fprintf(stderr, "    FFmpeg " COLOR_RED "failed" COLOR_RESET " with code %d!\n\n", ffmpegExitCode);
        FFExecute::addTextToFFOFile("    FFmpeg failed with code " + std::to_string(ffmpegExitCode) + "!\n\n");
    }
    else // no error - ffmpeg finished correctly
    {
        ++ m_correctlyPerformedFFmpegs;
        FFExecute::printProgress(m_duration);
        printf("\n");
        fprintf(stderr, "    FFmpeg " COLOR_GREEN "finished" COLOR_RESET "!\n\n");
        FFExecute::addTextToFFOFile("    FFmpeg finished!\n\n");
    }

}

void FFExecute::setTotalFFmpegsToPerform(int count)
{
    m_totalFFmpegsToPerform = count;
}

void FFExecute::runFFmpeg(cstr inFile, cstr outFile)
{
    FFExecute::openFFOFile();
    FFExecute::_runFFmpeg(inFile, outFile);
    FFExecute::closeFFOFile();
    ++ m_performedFFmpegs;
}