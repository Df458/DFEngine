#ifndef FONT_H
#define FONT_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <map>

class IScene;

struct Glyph
{
    char id = 0;
    glm::vec2 dimensions = {0, 0};
    glm::vec2 bearing = {0, 0 };
    float advance = 0;
    GLuint texture = 0;
};

class IFont
{
public:
    virtual ~IFont() {}
    virtual void cleanup(void) = 0;

    virtual void draw(IScene* scene, const char* text, glm::mat4 transform, float font_size = 16, glm::vec3 color = { 0, 0, 0 }) = 0;
    virtual void draw2D(IScene* scene, const char* text, glm::mat4 transform, float font_size = 16, glm::vec3 color = { 0, 0, 0 }) = 0;
    virtual bool failedLoad() = 0;
};

class Font : public IFont
{
public:
    Font(const char* filepath, FT_Library lib);
    virtual ~Font() {}
    void cleanup(void);

    void draw(IScene* scene, const char* text, glm::mat4 transform, float font_size = 16, glm::vec3 color = { 0, 0, 0 });
    void draw2D(IScene* scene, const char* text, glm::mat4 transform, float font_size = 16, glm::vec3 color = { 0, 0, 0 });
    virtual bool failedLoad() { return m_failed_load; }
protected:
    Glyph renderGlyph(char glyph, float font_size);

    FT_Face m_font_face;
    GLuint m_texture_uniform;
    GLuint m_color_uniform;
    GLuint m_vertex_position;
    GLuint m_transform_uniform;
    bool m_failed_load = true;
    std::map<float, Glyph> m_rendered_symbols[256];
};

class BitmapFont : public IFont
{
public:
    BitmapFont(const char* filepath);
    virtual ~BitmapFont() {}
    void cleanup(void);

    void draw(IScene* scene, const char* text, glm::mat4 transform, float font_size = 0, glm::vec3 color = { 0, 0, 0 });
    void draw2D(IScene* scene, const char* text, glm::mat4 transform, float font_size = 0, glm::vec3 color = { 0, 0, 0 });
    virtual bool failedLoad() { return m_failed_load; }
protected:
    GLuint m_texture;
    bool m_failed_load = true;
};

#endif
