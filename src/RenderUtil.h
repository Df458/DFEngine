#ifndef RENDER_UTIL
#define RENDER_UTIL
#include <GL/glew.h>
#include <string>

enum RenderPass
{
    FIRST_PASS = 0,
    STATIC_PASS = FIRST_PASS,
    DYNAMIC_PASS,
    LIGHTING_PASS,
    SKY_PASS,
    TRANSPARENT_PASS,
    HIDDEN_PASS,
    UI_PASS,
    LAST_PASS
};

const unsigned MAX_PARTICLES = 100000;

bool _checkGLError(const char* file, unsigned line);
#define checkGLError() _checkGLError(__FILE__, __LINE__)

extern GLuint WIREFRAME_PROGRAM;
extern GLuint SPRITE_PROGRAM;
extern GLuint PARTICLE_PROGRAM;
extern GLuint TEXT_PROGRAM;
extern GLuint QUAD_BUFFER;
extern GLuint BLANK_TEXTURE;

struct Texture
{
    GLuint texture_handle = 0;
    unsigned texture_width = 0;
    unsigned texture_height = 0;
    std::string name = "";
};

#endif
