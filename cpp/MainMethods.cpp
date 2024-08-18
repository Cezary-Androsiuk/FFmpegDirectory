#include "MainMethods.hpp"

str lastError;
const char possibleSeparators[] = {',', '/', '\\', /*'|', */';', '+', '?'};

vstr splitStringByChar(cstr str, char separator) 
{
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

fs::path createOutputFilename(cpath inFile, cpath outDirectory)
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

bool rm_all(const fs::path& path) {
    try {
        fs::remove_all(path);
        return true;
    } catch (const fs::filesystem_error& e) {
        fprintf(stderr, "Error while deleting structure: %s\n", e.what());
        return false;
    }
}