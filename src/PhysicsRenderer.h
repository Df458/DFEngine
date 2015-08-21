#ifndef PHYSICS_RENDERER_H
#define PHYSICS_RENDERER_H

#include <btBulletDynamicsCommon.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class IScene;

#define DEBUG_LINE_MAX 10000

class PhysicsRenderer : public btIDebugDraw
{
public:
    PhysicsRenderer();
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
    void batchDrawLines(void);
    void drawContactPoint(const btVector3& pointInB, const btVector3& normalOnB, btScalar distance, int lifetime, const btVector3& color);
    void reportErrorWarning(const char* warningString);
    void draw3dText(const btVector3& location, const char* textString);
    void setDebugMode(int debugMode) { m_debug_mode = debugMode; }
    int  getDebugMode() const { return m_debug_mode; }
    inline void setScene(IScene* scene) { u_scene = scene; }
protected:
    GLuint m_line_buffer;
    GLuint u_program;
    GLuint u_vertex_position;
    GLuint u_vertex_color;
    GLuint u_view_projection;
    IScene* u_scene;
    float m_line_count = 0;

    int m_debug_mode = 0;
};

#endif
