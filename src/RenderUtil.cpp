#include "RenderUtil.h"
#include "Util.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

bool _checkGLError(const char* file, unsigned line) {
    GLenum error;
    bool error_found = false;
    do {
        error = glGetError();
        if(error != GL_NO_ERROR) {
            error_found = true;
            //std::string message;

            switch(error) {
                case GL_INVALID_VALUE:
                    fprintf(stderr, "OpenGL error in %s, line %u: Parameter given is invalid.\n", file, line);
                    _warn(file, line, "OpenGL error: Invalid parameter");
                    break;
                case GL_INVALID_OPERATION:
                    fprintf(stderr, "OpenGL error in %s, line %u: Illegal operation attempted.\n", file, line);
                    _warn(file, line, "OpenGL error: Invalid operation");
                    break;
                case GL_INVALID_ENUM:
                    fprintf(stderr, "OpenGL error in %s, line %u: Enum given is invalid.\n", file, line);
                    _warn(file, line, "OpenGL error: Invalid enum");
                    break;
                default:
                    fprintf(stderr, "%s, %u: OpenGL error code 0x%0X. Consider adding an entry.\n", file, line, error);
                    _warn(file, line, "OpenGL error: Unknown");
            }

            //fprintf(stderr, "%s, %u: %s", file, line, message.c_str());
        }
    } while(error != GL_NO_ERROR);
    return error_found;
}
