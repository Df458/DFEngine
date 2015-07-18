#ifndef UTIL_H
#define UTIL_H
#include <string>

void _warn (const char* file, unsigned line, std::string message);
void _error(const char* file, unsigned line, std::string message, bool to_exit = true);
#define warn(message) _warn(__FILE__, __LINE__, message)
#define error(message) _error(__FILE__, __LINE__, message)

char* loadFileContents(std::string filepath);

#endif
