#include "Log.hpp"

std::string Log::m_sessionLogs = std::string();
bool Log::fileSizeProtectionExecuted = false;

void Log::info(std::string func, std::string log)
{
    Log::log( Log::buildPrefix(Log::time(), "I", func) + log );
}

void Log::warning(std::string func, std::string log)
{
    Log::log( Log::buildPrefix(Log::time(), "W", func) + log );
}

void Log::error(std::string func, std::string log)
{
    Log::log( Log::buildPrefix(Log::time(), "E", func) + log );
}

void Log::debug(std::string func, std::string log)
{
    Log::log( Log::buildPrefix(Log::time(), "D", func) + log );
}

std::string Log::time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm* time = localtime(&now);

    std::string buffer;
    sprintf(buffer.data(), "%d-%02d-%02d %02d:%02d:%02d.%03d",
        1900 + time->tm_year,
        1 + time->tm_mon,
        time->tm_mday,
        time->tm_hour,
        time->tm_min,
        time->tm_sec,
        tv.tv_usec / 1000
    );
    return buffer;
}

std::string Log::buildPrefix(std::string time, std::string type, std::string func)
{
    std::string prefix;
    // set time
    prefix = "[" + time +  "]" + " ";
    // set type
    prefix += type + " ";

    // set function name
    if(func.size() >= EST_FUNCTION_LENGTH)
        prefix += func;
    else
    {
        size_t fill = EST_FUNCTION_LENGTH - func.size();
        prefix += std::string(fill, SHORTER_FUNCTION_FILL_CHARACTER);
        prefix += func;
    }

#if SPACE_BETWEEN_CONTENT_SPACE_AND_CONTENT
    prefix += " ";
#endif

    // set space between content
    prefix += std::string(CONTENT_SPACE, CONTENT_SPACE_CHARACTER);

#if SPACE_BETWEEN_CONTENT_SPACE_AND_CONTENT
    prefix += " ";
#endif

    return prefix;
}

void Log::log(std::string content)
{
    Log::fileSizeProtection();

    Log::print(content);
    Log::saveFile(content);
    Log::addSession(content);
}

void Log::print(std::string content)
{
    printf("%s", content.c_str());
}

void Log::saveFile(std::string content)
{
    std::ofstream ofile(LOG_FILE, std::ios::app);
    if(!ofile.good())
    {
        fprintf(stderr, "Error while saving log to file!\n");
        return;
    }

    ofile << content;
    ofile.close();
}

void Log::addSession(std::string content)
{
    Log::m_sessionLogs += content + "\n";
}

void Log::fileSizeProtection()
{
    if(Log::fileSizeProtectionExecuted)
        return;

    Log::fileSizeProtectionExecuted = true;

    /// get log file size
    if(!std::filesystem::exists(LOG_FILE))
        return;
    long long fileSize = std::filesystem::file_size(LOG_FILE);

    /// compare with given def max size
    if(fileSize < MAX_LOG_FILE_SIZE)
        return;

    /// trim to given def size
    std::ifstream ifile(LOG_FILE);
    if(!ifile.good())
    {
        fprintf(stderr, "Error while oppening long file!\n");
        return;
    }
    ifile.seekg(fileSize - TRIM_LOG_FILE_SIZE);
    char trimmedData[TRIM_LOG_FILE_SIZE];
    ifile.read(trimmedData, TRIM_LOG_FILE_SIZE);
    ifile.close();
    
    std::filesystem::remove(LOG_FILE); // missing method moveToTrash, that exist in Qt :c

    std::ofstream ofile(LOG_FILE);
    if(!ofile.good())
    {
        fprintf(stderr, "Error while recreating, smaller log file!\n");
        return;
    }
    ofile.write(trimmedData, TRIM_LOG_FILE_SIZE);
    ofile.close();

    printf("Trimmed log file!\n");
}

std::string Log::Convert::vectorStringsToString(std::vector<std::string> list)
{
    std::string str("[");
    for(const auto &i : list)
        str += "\"" + i + "\", ";
    return str + "\b\b]";
}



