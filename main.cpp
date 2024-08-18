#include <cstdio>
#include <string>
#include <vector>
#include <filesystem>

#include "cpp/MainMethods.hpp"
#include "cpp/FFExecute.hpp"
#include "cpp/ListMaker.hpp"
#include "cpp/enums/SkipAction.hpp"

// compile:
// g++ main.cpp cpp\MainMethods.cpp cpp\FFExecute.cpp cpp\FFTester.cpp cpp\ListMaker.cpp -o ffmpegDir.exe

// instalation(add ffmpegAll.exe to PATH environment):
// 1 in windows search type "Edit the system environment variables" 
// 2 press "Environment Variables..." button
// 3 in bottom part (System variables) find variable named "Path" and double click on it
// 4 press on the right site the "New" button and type path to directory, where executable file (created after compilation) is located
// in example path to this executable (for now) is "D:\vscode\c++\projects\FFmpegDirectory"
// now, you can open cmd in any directory and just in command prompt type "ffmpegDir . mkv|mp4"


int main(int argc, const char **argv)
{
    printf("\n");

    fs::path directory;
    vstr extensions;
    SkipAction skipAction;
    if(!argsValid(argc, argv, &directory, &extensions, &skipAction))
    {
        fprintf(stderr, COLOR_RESET "Arguments are not valid:" COLOR_RED " %s\n" COLOR_RESET, lastError.c_str());
        fprintf(stderr, "Expected none, one or two arguments (directory, extensions)!\n");
        return 1;
    }

    printf("Selected directory: %s\n", directory.string().c_str());
    printf("Found files:\n");
    
    fs::path outDirectory( directory / "ffmpeg-h.265" );
    // printf(COLOR_RED "DEBUG ONLY!" COLOR_RESET "\n");
    // rm_all(outDirectory);
    if(!createOutputDirectory( outDirectory ))
    {
        fprintf(stderr, COLOR_RESET "Failed while creating output directory:" COLOR_RED " %s" COLOR_RESET, lastError.c_str());
        return 1;
    }

    vpath listOfFiles = ListMaker::listOfFiles(directory, extensions);

    FFExecute::setTotalFFmpegsToPerform(listOfFiles.size());
    FFExecute::setSkipAction(skipAction);
    
    str skippedText;
    if(skipAction == SkipAction::Skip) skippedText = "skipped";
    else if(skipAction == SkipAction::Copy) skippedText = "copied";
    else if(skipAction == SkipAction::Move) skippedText = "moved";

    printf("\n[ " COLOR_WHITE/*grey color*/ "correctlyPerformed / performed / totalToPerform   " 
        COLOR_RESET COLOR_RED "failed" COLOR_RESET " / " COLOR_YELLOW "%s" COLOR_RESET " ]\n", skippedText.c_str());

    str filesProgress = FFExecute::makeFileProgressPostfix();
    printf("Status: [ %s ]\n\n", filesProgress.c_str()); 

    for(const auto &inFile : listOfFiles)
    {
        // all files in list are valid at this point
        fs::path outFile = createOutputFilename(inFile, outDirectory);
        FFExecute::runFFmpeg(inFile.string(), outFile.string());
    }


    deleteDirectoryIfEmpty(outDirectory);

    printf("Finished all FFmpegs!\n");
}