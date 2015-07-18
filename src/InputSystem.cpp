#include "EventSystem.h"
#include "Game.h"
#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "Util.h"

InputSystem::InputSystem()
{
}

InputSystem::InputSystem(GraphicsSystem* gfx, IEventManager* events)
{
    u_gfx = gfx;
    u_events = events;
}

InputSystem::~InputSystem()
{
}

bool InputSystem::initialize(void)
{
    WindowData* window_data;
    window_data = static_cast<WindowData*>(glfwGetWindowUserPointer(u_gfx->m_main_window));
    window_data->input_data = this;

    glfwSetKeyCallback(u_gfx->m_main_window, keyCodeCallback);
    glfwSetMouseButtonCallback(u_gfx->m_main_window, cursorButtonCallback);
    glfwSetCursorPosCallback(u_gfx->m_main_window, cursorMotionCallback);
    glfwSetCursorEnterCallback(u_gfx->m_main_window, cursorEnterCallback);
    glfwSetScrollCallback(u_gfx->m_main_window, cursorScrollCallback);
    glfwSetDropCallback(u_gfx->m_main_window, fileDropCallback);

    return true;
}

void InputSystem::cleanup(void)
{
}

void InputSystem::pushGameEvents(void)
{
}

DFBaseInputSystem::DFBaseInputSystem(GraphicsSystem* gfx, IEventManager* events) : InputSystem(gfx, events)
{
}

DFBaseInputSystem::~DFBaseInputSystem(void)
{
}

void DFBaseInputSystem::pushGameEvents(void)
{
}

void DFBaseInputSystem::update(void)
{
    for(int i = 0; i < GLFW_KEY_LAST; ++i)
        if(m_keys[i] == GLFW_PRESS)
            m_keys[i] = GLFW_REPEAT;
        else if(m_keys[i] == GLFW_RELEASE)
            m_keys[i] = 3;
    for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i)
        if(m_mouse_buttons[i] == GLFW_PRESS)
            m_mouse_buttons[i] = GLFW_REPEAT;
        else if(m_mouse_buttons[i] == GLFW_RELEASE)
            m_mouse_buttons[i] = 3;
    m_scroll_delta.x = 0;
    m_scroll_delta.y = 0;
}

void keyCodeCallback(GLFWwindow* window, int key_code, int scancode, int action, int mod)
{
    InputSystem* input = static_cast<WindowData*>(glfwGetWindowUserPointer(window))->input_data;
    // TODO: Add modifier support, press/release callbacks vs down
    if(action == GLFW_PRESS || action == GLFW_RELEASE)
        input->m_keys[key_code] = action;
}

void cursorEnterCallback(GLFWwindow* window, int entered)
{
    InputSystem* input = static_cast<WindowData*>(glfwGetWindowUserPointer(window))->input_data;
    input->m_mouse_present = entered;
}

void cursorMotionCallback(GLFWwindow* window, double pos_x, double pos_y)
{
    InputSystem* input = static_cast<WindowData*>(glfwGetWindowUserPointer(window))->input_data;
    input->m_mouse_delta.x = pos_x - input->m_mouse_position.x;
    input->m_mouse_delta.y = pos_y - input->m_mouse_position.y;
    input->m_mouse_position.x = pos_x;
    input->m_mouse_position.y = pos_y;
}

void cursorButtonCallback(GLFWwindow* window, int button, int action, int mod)
{
    InputSystem* input = static_cast<WindowData*>(glfwGetWindowUserPointer(window))->input_data;
    // TODO: Add modifier support, press/release callbacks vs down
    if(action == GLFW_PRESS || action == GLFW_RELEASE)
        input->m_mouse_buttons[button] = action;
}

void cursorScrollCallback(GLFWwindow* window, double delta_x, double delta_y)
{
    InputSystem* input = static_cast<WindowData*>(glfwGetWindowUserPointer(window))->input_data;
    input->m_scroll_delta.x += delta_x;
    input->m_scroll_delta.y += delta_y;
}

void fileDropCallback(GLFWwindow* window, int file_count, const char** file_list)
{
    warn("File drop support is not yet implemented!");
}

int input_keystate(lua_State* state)
{
    if(lua_isstring(state, 1)) {
        char key_char = lua_tostring(state, 1)[0];
        if(key_char >= 97 && key_char <= 122)
            key_char -= 32;
        lua_pushinteger(state, g_game->input()->getKeyState(key_char));
    } else if(lua_isinteger(state, 1)) {
        int key_id = lua_tointeger(state, 1);
        lua_pushinteger(state, g_game->input()->getKeyState(key_id));
    } else {
        return luaL_error(state, "keystate expects a char or int.");
    }
    return 1;
}

int input_mousestate(lua_State* state)
{
    if(lua_isinteger(state, 1)) {
        int button_id = lua_tointeger(state, 1);
        lua_pushinteger(state, g_game->input()->getMouseState(button_id));
    } else {
        return luaL_error(state, "mousestate expects an int.");
    }
    return 1;
}

int input_mouseposition(lua_State* state)
{
    glm::vec2 position = g_game->input()->getMousePosition();
    lua_pushnumber(state, position.x);
    lua_pushnumber(state, position.y);
    return 2;
}

int input_mousescroll(lua_State* state)
{
    glm::vec2 scroll = g_game->input()->getMouseScroll();
    lua_pushnumber(state, scroll.x);
    lua_pushnumber(state, scroll.y);
    return 2;
}
