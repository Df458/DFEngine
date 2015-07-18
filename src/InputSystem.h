#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H
#include "System.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

class IEventManager;
class GraphicsSystem;

class InputSystem : public ISystem
{
public:
    InputSystem();
    InputSystem(GraphicsSystem* gfx, IEventManager* events);
    virtual ~InputSystem(void);
    virtual bool initialize(void);
    virtual void update(void) = 0;
    virtual void cleanup(void);
    virtual void pushGameEvents(void);
    inline int getKeyState(char key_char) const { return m_keys[(int)key_char]; }
    inline int getKeyState(int key_id) const { return (key_id < GLFW_KEY_LAST) ? m_keys[key_id] : -1; }
    inline int getMouseState(int button_id) const { return (button_id < GLFW_MOUSE_BUTTON_LAST) ? m_mouse_buttons[button_id] : -1; }
    inline glm::vec2 getMousePosition(void) const { return m_mouse_position; }
    inline glm::vec2 getMouseScroll(void) const { return m_scroll_delta; }

    friend void keyCodeCallback(GLFWwindow*, int, int, int, int);
    friend void cursorMotionCallback(GLFWwindow*, double, double);
    friend void cursorEnterCallback(GLFWwindow*, int);
    friend void cursorButtonCallback(GLFWwindow*, int, int, int);
    friend void cursorScrollCallback(GLFWwindow*, double, double);
protected:
    GraphicsSystem* u_gfx;
    IEventManager* u_events;

    bool      m_mouse_present = false;
    glm::vec2 m_mouse_position;
    glm::vec2 m_mouse_delta;
    glm::vec2 m_scroll_delta;
    int m_mouse_buttons[GLFW_MOUSE_BUTTON_LAST] = { 3 };
    int m_keys[GLFW_KEY_LAST] = { 3 };
};

class DFBaseInputSystem : public InputSystem
{
public:
    DFBaseInputSystem(GraphicsSystem* gfx, IEventManager* events);
    virtual ~DFBaseInputSystem(void);
    virtual void update(void);
    virtual void pushGameEvents(void) final;

protected:
};

void keyCodeCallback(GLFWwindow* window, int key_code, int scancode, int action, int mod);

void cursorEnterCallback(GLFWwindow* window, int entered);
void cursorMotionCallback(GLFWwindow* window, double delta_x, double delta_y);
void cursorButtonCallback(GLFWwindow* window, int button, int action, int mod);
void cursorScrollCallback(GLFWwindow* window, double delta_x, double delta_y);

void fileDropCallback(GLFWwindow* window, int file_count, const char** file_list);

int input_keystate(lua_State* state);
int input_mousestate(lua_State* state);
int input_mouseposition(lua_State* state);
int input_mousescroll(lua_State* state);

const luaL_Reg input_funcs[] =
{
    {"key", input_keystate},
    {"mouse_button", input_mousestate},
    {"mouse_position", input_mouseposition},
    {"mouse_scroll", input_mousescroll},
    {0, 0}
};

#endif
