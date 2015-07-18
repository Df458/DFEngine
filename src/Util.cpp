#include "Util.h"

#include <cstdio>
#include <cstdlib>

void _warn(const char* file, unsigned line, std::string message) {
    fprintf(stderr, "%s:%d, WARNING: %s\n", file, line, message.c_str());
}

void _error(const char* file, unsigned line, std::string message, bool to_exit) {
    fprintf(stderr, "%s:%d, FATAL ERROR: %s\n", file, line, message.c_str());
    if(to_exit)
        exit(1);
}

char* loadFileContents(std::string filepath) {
    FILE* file = fopen(filepath.c_str(), "rb");
    if(!file) {
        error("Failed to load " + filepath + ": File not found.");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* filedata = new char[filesize + 1];
    fread(filedata, 1, filesize, file);
    filedata[filesize] = '\0';

    fclose(file);
    return filedata;
}
