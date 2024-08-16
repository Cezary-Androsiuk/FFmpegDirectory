#include <cstdio>
#include <string>
#include <vector>
#include <filesystem>

#include <unistd.h>
#include <fcntl.h>

#include "cpp/FFExecute.hpp"
#include "cpp/ListMaker.hpp"

// compile:
// g++ main.cpp cpp\FFExecute.cpp cpp\FFTester.cpp cpp\ListMaker.cpp -o ffmpegAll.exe

// instalation(add ffmpegAll.exe to PATH environment):
// to do this,
// 1 in windows search type "Edit the system environment variables" 
// 2 press "Environment Variables..." button
// 3 in bottom part (System variables) find variable named "Path" and double click on it
// 4 press on the right site "New" button and type path to directory, where executable file (created after compilation) is located
// in example path to this executable (for now) is "D:\vscode\c++\just_small_projects\ffmpeg_all_in_given_directory"

namespace fs = std::filesystem;
typedef std::string str;
typedef std::vector<str> vstr;
typedef const fs::path &cpath;
typedef std::vector<fs::path> vpath;


str lastError;
#define FUNC_START {lastError = "";}

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"


bool argsValid(int argc, const char **argv, fs::path * const absoluteDirectory = nullptr)
{
    FUNC_START

    fs::path givenDirectory = argc == 1 ? fs::current_path() : argv[1];

    if(!fs::exists( givenDirectory ))
    {
        lastError = "File " + givenDirectory.string() + " not exist!";
        return false;
    }

    if(!fs::is_directory( givenDirectory ))
    {
        lastError = "File " + givenDirectory.string() + " is not a directory!";
        return false;
    }

    if(absoluteDirectory != nullptr)
        *absoluteDirectory = fs::absolute( givenDirectory );

    return true;

    // nice in tests, shows all what is needed
    // std::string currentPath = fs::current_path().string();
    // printf("current path: %s\n", currentPath.c_str());
    // fs::path selectedPath(argv[1]);
    // printf("is relative: %d\nis absolute: %d\n", selectedPath.is_relative(), selectedPath.is_absolute());
    // printf("exist: %d\n", fs::exists(selectedPath));
    // printf("is directory: %d\n", fs::is_directory(selectedPath));
    // printf("selected path: %s\n", selectedPath.string().c_str());
    // printf("absolute: %s\n",fs::absolute(selectedPath).string().c_str());
}

bool isDirectoryEmpty(fs::path directory)
{
    // should be an directory at this point, then no checking

    // very fancy way to check it xd (by Gemini)
    return fs::directory_iterator(directory) == fs::directory_iterator();
}

bool createOutputDirectory(fs::path outDirectory)
{
    FUNC_START

    if(fs::exists( outDirectory ))
    {
        // "ffmpeg-h.265" exist, now check if is directory
        if(!fs::is_directory( outDirectory ))
        {
            lastError = "Output directory already exist, but is not a directory: " + outDirectory.string();
            return false;
        }

        if(!isDirectoryEmpty( outDirectory ))
        {
            // this will help to avoid multiple program execution in the same directory
            lastError = "Output directory already exist, and is not empty: " + outDirectory.string();
            return false;
        }
    }
    else
    {
        // "ffmpeg-h.265" not exist, now try to create it
        if(!fs::create_directory( outDirectory ))
        {
            lastError = "Failed while creating output directory: " + outDirectory.string();
            return false;
        }
    }

    return true;
}

fs::path createOutputFile(cpath inFile, cpath outDirectory)
{
    str filename = inFile.filename().string();

    size_t dotPos = filename.find_last_of('.');
    return outDirectory / (filename.substr(0, dotPos+1) + "mp4");
}

void deleteDirectoryIfEmpty(fs::path outDirectory)
{
    if(fs::exists( outDirectory ))
    {
        // "ffmpeg-h.265" exist, now check if is directory
        if(!fs::is_directory( outDirectory ))
            return;

        if(isDirectoryEmpty( outDirectory ))
        {
            fs::remove( outDirectory );
        }
    }
}
 
int main(int argc, const char **argv)
{
    /// add stronly attached arguments to program, where extensions are second argument for example:
    /// mkv/mp4
    /// mkv+mp4
    /// mkv|mp4

    fs::path directory;
    if(!argsValid(argc, argv, &directory))
    {
        fprintf(stderr, COLOR_RESET "Arguments are not valid:" COLOR_RED " %s\n" COLOR_RESET, lastError.c_str());
        fprintf(stderr, "Expected none or one argument, which is path to directory!\n");
        return 1;
    }

    printf("selected directory: %s\n", directory.string().c_str());
    
    fs::path outDirectory( directory / "ffmpeg-h.265" );
    if(!createOutputDirectory( outDirectory ))
    {
        fprintf(stderr, COLOR_RESET "Failed while creating output directory:" COLOR_RED " %s" COLOR_RESET, lastError.c_str());
        return 1;
    }


    vstr acceptableExtensions = {"mkv", "mp4"};
    vpath listOfFiles = ListMaker::listOfFiles(directory, acceptableExtensions);

    FFExecute::setTotalFFmpegsToPerform(listOfFiles.size());

    for(const auto &inFile : listOfFiles)
    {
        // all files in list are valid at this point
        fs::path outFile = createOutputFile(inFile, outDirectory);
        // FFExecute::runFFmpeg(inFile.string(), outFile.string());
    }

    deleteDirectoryIfEmpty(outDirectory);

    printf("Finished all FFmpegs!\n");
    // correctly_performed_ffmpegs / performed_ffmpegs / total_ffmpegs_to_perform   failed_ffmpegs / skipped_ffmpegs
    str filesProgress = FFExecute::makeFileProgressPostfix();
    printf("Summary: [ " COLOR_WHITE/*grey color*/ "%s" COLOR_RESET " ]\n", filesProgress.c_str()); 

}