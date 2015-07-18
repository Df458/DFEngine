#include "ResourceDefines.h"
#include "ResourceManager.h"
#include "Game.h"

#include "Font.h"
#include "RenderUtil.h"
#include "Scene.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <math.h>

Font::Font(const char* filename, FT_Library lib)
{
    if(FT_New_Face(lib, filename, 0, &m_font_face))
        return; // TODO: Print errors
    m_vertex_position = glGetAttribLocation(TEXT_PROGRAM, "vertex_pos");
    m_transform_uniform = glGetUniformLocation(TEXT_PROGRAM, "model_view_projection");
    m_texture_uniform = glGetUniformLocation(TEXT_PROGRAM, "texture");
    m_color_uniform = glGetUniformLocation(TEXT_PROGRAM, "color");
    checkGLError();

    m_failed_load = false;
}

void Font::cleanup(void)
{
    FT_Done_Face(m_font_face);
}

void Font::draw(IScene* scene, const char* text, glm::mat4 transform, float font_size, glm::vec3 color)
{
	FT_Set_Char_Size(m_font_face, 0, font_size * 64, 300, 300);
    glUseProgram(TEXT_PROGRAM);
    checkGLError();

    glm::vec3 pen(0, 0, 0);
    for(const char* i = text; i[0]; ++i) {
        Glyph glyph = renderGlyph(i[0], font_size);

        if(i[0] == '\n') {
            pen.x = 0;
            pen.y += font_size * 4;
            continue;
        } else if(i[0] == ' ' || glyph.id == 0) {
            pen.x += font_size * 2;
            continue;
        }

        glUniform3f(m_color_uniform, color.x, color.y, color.z);
        glm::vec3 offset(glyph.bearing.x, -glyph.bearing.y, 0.0f);
        glm::mat4 mvp = scene->getActiveProjectionMatrix() * scene->getActiveViewMatrix() * transform * glm::translate(glm::mat4(1), -(pen + offset) / 300.0f) * glm::scale(glm::mat4(1), glm::vec3(glyph.dimensions.x / 300.0f, glyph.dimensions.y / 300.0f, 1.0f));
        glUniformMatrix4fv(m_transform_uniform, 1, GL_FALSE, &mvp[0][0]);
        glEnableVertexAttribArray(m_vertex_position);
        glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
        glVertexAttribPointer(m_vertex_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyph.texture);
        glUniform1i(m_texture_uniform, 0);
        checkGLError();

        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkGLError();

        glDisableVertexAttribArray(m_vertex_position);
        pen.x += glyph.advance;
        //fprintf(stderr, "(%ld)\n", (int)glyph.advance);
    }
}

void Font::draw2D(IScene* scene, const char* text, glm::mat4 transform, float font_size, glm::vec3 color)
{
	FT_Set_Pixel_Sizes(m_font_face, 0, font_size);

    glUseProgram(TEXT_PROGRAM);

    glm::vec3 pen(font_size, font_size, 0);
    glm::vec2 view = scene->getViewportSize();
    for(const char* i = text; i[0]; ++i) {
        Glyph glyph = renderGlyph(i[0], font_size);
        //FT_UInt char_index = FT_Get_Char_Index(m_font_face, i[0]);

        //if(FT_Load_Glyph(m_font_face, char_index, FT_LOAD_DEFAULT) || FT_Render_Glyph(m_font_face->glyph, FT_RENDER_MODE_NORMAL))
            //continue; // TODO: Handle FreeType errors

        if(i[0] == '\n') {
            pen.x = font_size;
            pen.y += font_size;
            continue;
        } else if(i[0] == ' ' || glyph.id == 0) {
            pen.x += font_size;
            continue;
        }

        //FT_Bitmap& bitmap = m_font_face->glyph->bitmap;

        glUniform3f(m_color_uniform, color.x, color.y, color.z);
        glm::vec3 offset(glyph.bearing.x / 64, -glyph.bearing.y / 128, 0.0f);
        glm::mat4 mvp = glm::ortho(0.0f, view.x, view.y, 0.0f, -10.0f, 10.0f) * transform * glm::rotate(glm::mat4(1), (float)M_PI, glm::vec3(0, 0, 1)) * glm::translate(glm::mat4(1), -(pen + offset)) * glm::scale(glm::mat4(1), glm::vec3(glyph.dimensions.x, glyph.dimensions.y, 1.0f));
        glUniformMatrix4fv(m_transform_uniform, 1, GL_FALSE, &mvp[0][0]);
        glEnableVertexAttribArray(m_vertex_position);
        glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
        glVertexAttribPointer(m_vertex_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glyph.texture);
        glUniform1i(m_texture_uniform, 0);
        checkGLError();

        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkGLError();

        glDisableVertexAttribArray(m_vertex_position);
        pen.x += glyph.advance / 64;
    }
}

Glyph Font::renderGlyph(char glyph, float font_size)
{
    if(m_rendered_symbols[(int)glyph].find(font_size) != m_rendered_symbols[(int)glyph].end()) {
        return m_rendered_symbols[(int)glyph].at(font_size);
    }
    Glyph new_glyph;

    FT_UInt char_index = FT_Get_Char_Index(m_font_face, glyph);

    if(FT_Load_Glyph(m_font_face, char_index, FT_LOAD_DEFAULT) || FT_Render_Glyph(m_font_face->glyph, FT_RENDER_MODE_NORMAL))
        return new_glyph;

    FT_Bitmap& bitmap   = m_font_face->glyph->bitmap;
    new_glyph.bearing.x = m_font_face->glyph->metrics.horiBearingX / 64;
    new_glyph.bearing.y = m_font_face->glyph->metrics.horiBearingY / 128;
    new_glyph.advance   = m_font_face->glyph->metrics.horiAdvance / 64;

    glGenTextures(1, &new_glyph.texture);
    glBindTexture(GL_TEXTURE_2D, new_glyph.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkGLError();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width, bitmap.rows, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap.buffer);
    checkGLError();
    new_glyph.id = glyph;
    new_glyph.dimensions = glm::vec2(bitmap.width, bitmap.rows);
    m_rendered_symbols[(int)glyph][font_size] = new_glyph;

    return new_glyph;
}
