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
const char possibleSeparators[] = {',', '/', '\\', '|', ';', '+', '?'};
#define FUNC_START {lastError = "";}
#define DEFAULT_PATH fs::current_path()
#define DEFAULT_EXTENSIONS {"mkv", "mp4"}

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_WHITE   "\033[37m"

#include <iostream>
#include <string>
#include <vector>

vstr splitStringByChar(cstr str, char separator) {
    // by Gemini
    vstr splited;
    size_t start = 0;
    size_t end = str.find(separator);

    while (end != str::npos) {
        splited.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(separator, start);
    }

    splited.push_back(str.substr(start, str::npos));

    return splited;
}

vstr splitExtensionsInput(str input)
{
    char usedSeparator;
    for(int i=0; i<sizeof(possibleSeparators); i++)
    {
        char testedSeparator = possibleSeparators[i];
        if(input.find_first_of(testedSeparator) != str::npos)
        {
            usedSeparator = testedSeparator;
            break;
        }
    }
    return splitStringByChar(input, usedSeparator);
}

bool argsValid(int argc, const char **argv, fs::path *const directory, vstr *const extensions)
{
    FUNC_START

    fs::path givenDirectory;
    vstr givenExtensions;

    if(argc == 1) // none args
    {
        givenDirectory = DEFAULT_PATH;
        givenExtensions = DEFAULT_EXTENSIONS;
    }
    else if(argc == 2) // one argument
    {
        givenDirectory = argv[1];
        givenExtensions = DEFAULT_EXTENSIONS;
    }
    else if(argc >= 3) // two or more arguments
    {
        givenDirectory = argv[1];
        givenExtensions = splitExtensionsInput( str(argv[2]) );
    }
    
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

    if(directory != nullptr)
        *directory = fs::absolute( givenDirectory );

    if(extensions != nullptr)
        *extensions = givenExtensions;

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
    printf("\n");

    fs::path directory;
    vstr extensions;
    if(!argsValid(argc, argv, &directory, &extensions))
    {
        fprintf(stderr, COLOR_RESET "Arguments are not valid:" COLOR_RED " %s\n" COLOR_RESET, lastError.c_str());
        fprintf(stderr, "Expected none, one or two arguments (directory, extensions)!\n");
        return 1;
    }

    printf("selected directory: %s\n", directory.string().c_str());
    
    fs::path outDirectory( directory / "ffmpeg-h.265" );
    if(!createOutputDirectory( outDirectory ))
    {
        fprintf(stderr, COLOR_RESET "Failed while creating output directory:" COLOR_RED " %s" COLOR_RESET, lastError.c_str());
        return 1;
    }

    printf("[ " COLOR_WHITE/*grey color*/ "correctlyPerformed / performed / totalToPerform   failed / skipped" COLOR_RESET " ]\n\n");

    vstr acceptableExtensions = {"mkv", "mp4"};
    vpath listOfFiles = ListMaker::listOfFiles(directory, acceptableExtensions);

    FFExecute::setTotalFFmpegsToPerform(listOfFiles.size());

    for(const auto &inFile : listOfFiles)
    {
        // all files in list are valid at this point
        fs::path outFile = createOutputFile(inFile, outDirectory);
        FFExecute::runFFmpeg(inFile.string(), outFile.string());
    }

    deleteDirectoryIfEmpty(outDirectory);

    printf("Finished all FFmpegs!\n");
    // correctly_performed_ffmpegs / performed_ffmpegs / total_ffmpegs_to_perform   failed_ffmpegs / skipped_ffmpegs
    str filesProgress = FFExecute::makeFileProgressPostfix();
    printf("Summary: [ " COLOR_WHITE/*grey color*/ "%s" COLOR_RESET " ]\n", filesProgress.c_str()); 

}