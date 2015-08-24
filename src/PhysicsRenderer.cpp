#include "Color.h"
#include "Font.h"
#include "Game.h"
#include "GraphicsSystem.h"
#include "ResourceManager.h"
#include "PhysicsRenderer.h"
#include "RenderUtil.h"
#include "Scene.h"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>

PhysicsRenderer::PhysicsRenderer()
{
    glGenBuffers(1, &m_line_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float) * DEBUG_LINE_MAX, NULL, GL_STREAM_DRAW);

    u_program = WIREFRAME_PROGRAM;
    u_vertex_position = glGetAttribLocation(u_program, "vertex_pos");
    u_vertex_color = glGetAttribLocation(u_program, "vertex_color");
    u_view_projection = glGetUniformLocation(u_program, "view_projection");
    checkGLError();
}

void PhysicsRenderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if(m_line_count == DEBUG_LINE_MAX) {
        warn("Attempting to draw lines, but the buffer is full!");
        return;
    }
    float line[12];
    line[0]  = from.x();
    line[1]  = from.y();
    line[2]  = from.z();
    line[3]  = color.x();
    line[4]  = color.y();
    line[5]  = color.z();
    line[6]  = to.x();
    line[7]  = to.y();
    line[8]  = to.z();
    line[9]  = color.x();
    line[10] = color.y();
    line[11] = color.z();
    glUseProgram(u_program);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 12 * sizeof(float) * m_line_count, 12 * sizeof(float), line);
    // Send to the shader and render
    checkGLError();
    //glUniformMatrix4fv(u_view_projection, 1, GL_FALSE, &(u_scene->getActiveProjectionMatrix() * u_scene->getActiveViewMatrix())[0][0]);
    //glEnableVertexAttribArray(u_vertex_position);
    //glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);
    //glVertexAttribPointer(u_vertex_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //checkGLError();

    //glEnableVertexAttribArray(u_vertex_color);
    //glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);
    //glVertexAttribPointer(u_vertex_color, 3, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(GL_FLOAT)));
    //checkGLError();

    //glDrawArrays(GL_LINES, 0, 2);

    //glDisableVertexAttribArray(u_vertex_position);
    //glDisableVertexAttribArray(u_vertex_color);

    m_line_count++;
}

void PhysicsRenderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
    if(m_line_count == DEBUG_LINE_MAX) {
        warn("Attempting to draw lines, but the buffer is full!");
        return;
    }
    float line[12];
    line[0]  = from.x();
    line[1]  = from.y();
    line[2]  = from.z();
    line[3]  = fromColor.x();
    line[4]  = fromColor.y();
    line[5]  = fromColor.z();
    line[6]  = to.x();
    line[7]  = to.y();
    line[8]  = to.z();
    line[9]  = toColor.x();
    line[10] = toColor.y();
    line[11] = toColor.z();
    glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 12 * sizeof(float) * m_line_count, 12 * sizeof(float), line);

    m_line_count++;
}

void PhysicsRenderer::batchDrawLines(void)
{
    glUseProgram(u_program);
    checkGLError();

    glUniformMatrix4fv(u_view_projection, 1, GL_FALSE, &(u_scene->getActiveProjectionMatrix() * u_scene->getActiveViewMatrix())[0][0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_buffer);

    glEnableVertexAttribArray(u_vertex_position);
    glVertexAttribPointer(u_vertex_position, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)0);
    checkGLError();

    glEnableVertexAttribArray(u_vertex_color);
    glVertexAttribPointer(u_vertex_color, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
    checkGLError();

    glDrawArrays(GL_LINES, 0, 2 * m_line_count);

    glDisableVertexAttribArray(u_vertex_position);
    glDisableVertexAttribArray(u_vertex_color);
    m_line_count = 0;
}

void PhysicsRenderer::drawContactPoint(const btVector3& pointInB, const btVector3& normalOnB, btScalar distance, int lifetime, const btVector3& color)
{
    warn("PhysicsRenderer::drawContactPoint is unimplimented!");
}

void PhysicsRenderer::reportErrorWarning(const char* warningString)
{
    warn(warningString);
}

void PhysicsRenderer::draw3dText(const btVector3& location, const char* textString)
{
    g_game->resources()->getFont("FiraSans-Medium")->draw(g_game->graphics()->getActiveScene(), textString, glm::translate(glm::mat4(1), glm::vec3(location.x(), location.y(), location.z())), 16, Color::White);
}
