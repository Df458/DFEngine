#include "Scene.h"
#include "SceneNode.h"
#include "Shader.h"
#include "Util.h"

BasicShader::BasicShader(GLuint program)
{
    m_program = program;
    m_vertex_position_attrib = glGetAttribLocation(m_program, "vertex_pos");
    m_vertex_uv_attrib = glGetAttribLocation(m_program, "vertex_uv");
    m_vertex_normal_attrib = glGetAttribLocation(m_program, "vertex_normal");
    m_w_uniform = glGetUniformLocation(m_program, "world");
    m_vp_uniform = glGetUniformLocation(m_program, "view_projection");
    m_color_uniform = glGetUniformLocation(m_program, "color");
    m_texture_uniform = glGetUniformLocation(m_program, "texture");
    checkGLError();
}

BasicShader::~BasicShader(void)
{
}

void BasicShader::cleanup(void)
{
    glDeleteProgram(m_program);
}

void BasicShader::prepareForRender(IScene* scene, IModel* model, glm::mat4 world_matrix, Texture* texture)
{
    glUseProgram(m_program);
    checkGLError();

    glm::mat4 vp_matrix = scene->getActiveProjectionMatrix() * scene->getActiveViewMatrix();
    glm::mat4 world_matrix_final = scene->getMatrix() * world_matrix;
    glUniformMatrix4fv(m_vp_uniform, 1, GL_FALSE, &vp_matrix[0][0]);
    glUniformMatrix4fv(m_w_uniform, 1, GL_FALSE, &world_matrix[0][0]);
    glUniform4f(m_color_uniform, 1.0f, 0.7f, 0.0f, 1.0f);
    checkGLError();

    glEnableVertexAttribArray(m_vertex_position_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, model->getVertices());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getIndices());
    glVertexAttribPointer(m_vertex_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    glEnableVertexAttribArray(m_vertex_uv_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, model->getUVs());
    glVertexAttribPointer(m_vertex_uv_attrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    glEnableVertexAttribArray(m_vertex_normal_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, model->getNormals());
    glVertexAttribPointer(m_vertex_normal_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    glActiveTexture(GL_TEXTURE0);
    if(!texture)
        glBindTexture(GL_TEXTURE_2D, BLANK_TEXTURE);
    else
        glBindTexture(GL_TEXTURE_2D, texture->texture_handle);
    glUniform1i(m_texture_uniform, 0);
    checkGLError();
}

void BasicShader::postRender(void)
{
    glDisableVertexAttribArray(m_vertex_position_attrib);
    glDisableVertexAttribArray(m_vertex_normal_attrib);
    glDisableVertexAttribArray(m_vertex_uv_attrib);
}
