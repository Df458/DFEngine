#include "Util.h"

#include <cstdio>
#include <cstdlib>
#ifdef WINDOWS
	#include <windows.h>
#else
    #include <unistd.h>
#endif

FILE* log = NULL;
void init_log() {
    log = fopen((getPath() + "/log").c_str(), "w");
    if(!log) {
        log = stderr;
        warn("Failed to open log file!");
    }
}

void _warn(const char* file, unsigned line, std::string message)
{
    if(!log)
        init_log();
    fprintf(log, "%s:%d, WARNING: %s\n", file, line, message.c_str());
}

void _error(const char* file, unsigned line, std::string message, bool to_exit)
{
    if(!log)
        init_log();
    fprintf(log, "%s:%d, FATAL ERROR: %s\n", file, line, message.c_str());
    if(to_exit)
        exit(1);
}

char* loadFileContents(std::string filepath)
{
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

std::string getPath()
{
	char buf[1024];
#ifdef WINDOWS
    GetModuleFileName(NULL, buf, 1024);
    std::string::size_type pos = std::string(buf).find_last_of("\\/");
    return std::string(buf).substr(0, pos);
#elif __GNUC__
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len != -1) {
        buf[len] = '\0';
        std::string s = std::string(buf);
        return s.erase(s.rfind("/"));
    }else{
		error("Error: Could not determine working directory\n");
		return NULL;
    }
#endif
}
