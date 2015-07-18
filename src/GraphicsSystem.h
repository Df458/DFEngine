#ifndef GRAPHICS_SYSTEM_H
#define GRAPHICS_SYSTEM_H

#include "PhysicsRenderer.h"
#include "System.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class IScene;
class Level;
class InputSystem;
class PhysicsRenderer;

class IGraphics {
public:
    virtual ~IGraphics(void) = 0;
    virtual void render(void) const = 0;
    virtual void setTitle(const char* title) = 0;
    virtual void loadSceneFromLevel(const Level* level) = 0;
    virtual void updateViewportSize(int width, int height) = 0;
};

inline IGraphics::~IGraphics() {}

class GraphicsSystem : public IGraphics, public ISystem {
public:
    GraphicsSystem(void);
    virtual ~GraphicsSystem(void);
    virtual bool initialize(void);
    virtual void render(void) const;
    virtual void cleanup(void);
    virtual void setTitle(const char* title) final;
    virtual void loadSceneFromLevel(const Level* level);
    virtual void updateViewportSize(int width, int height);
    inline IScene* getActiveScene(void) { return m_active_scene; }
    void setPhysicsDebug(PhysicsRenderer* pr) { m_physics_debug = pr; m_physics_debug->setScene(m_active_scene); }
    PhysicsRenderer* getPhysicsDebug(void) { return m_physics_debug; }

    friend class InputSystem;
    friend void sizeCallback(GLFWwindow* win, int width, int height);
private:
    GLFWwindow* m_main_window = 0;
    IScene* m_active_scene = 0;
    PhysicsRenderer* m_physics_debug = 0;
};

struct WindowData {
    GraphicsSystem* gfx_data;
    InputSystem*    input_data;
};

void closeCallback(GLFWwindow* win);
void sizeCallback(GLFWwindow* win, int width, int height);

#endif