#include "CGraphics.h"
#include "EventSystem.h"
#include "Game.h"
#include "GraphicsSystem.h"
#include "Level.h"
#include "PhysicsRenderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Util.h"
#include <functional>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std::placeholders;

GraphicsSystem::GraphicsSystem(void)
{

}

GraphicsSystem::~GraphicsSystem(void)
{
}

bool GraphicsSystem::initialize()
{
    //glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_main_window = glfwCreateWindow(800, 600, "Test Window", NULL, NULL);
    if(!m_main_window) {
        error("Unable to create window.");
        return false;
    }
    glfwSetWindowCloseCallback(m_main_window, closeCallback);
    glfwSetFramebufferSizeCallback(m_main_window, sizeCallback);
    WindowData* window_data = new WindowData;
    window_data->gfx_data = this;
    glfwSetWindowUserPointer(m_main_window, window_data);

    glfwMakeContextCurrent(m_main_window);

	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
        error("glewInit() failed.");
        glfwDestroyWindow(m_main_window);
        return false;
    }
    glGetError(); // Because GLEW is silly. <http://stackoverflow.com/questions/20034615/why-does-glewinit-result-in-gl-invalid-enum-after-making-some-calls-to-glfwwin>
    GLuint VAO; // FIXME: Stupid Hack. <http://stackoverflow.com/questions/13403807/glvertexattribpointer-raising-gl-invalid-operation>
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_active_scene = new Scene();

    auto gfx_callback = std::bind(&Scene::CGraphicsCreatedCallback, static_cast<Scene*>(m_active_scene), _1);
    g_game->events()->addSubscription(Callback(this, gfx_callback), CGraphicsCreatedEvent::m_type);
    auto destr_callback = std::bind(&Scene::actorRemovedCallback, static_cast<Scene*>(m_active_scene), _1);
    g_game->events()->addSubscription(Callback(this, destr_callback), ActorDestroyedEvent::m_type);

    return true;
}

void GraphicsSystem::init_letterbox(void)
{
    m_letterbox_vertex_attrib = glGetAttribLocation(LETTERBOX_PROGRAM, "vertex_pos");
    m_letterbox_translation_uniform = glGetUniformLocation(LETTERBOX_PROGRAM, "translation");
    m_letterbox_scale_uniform = glGetUniformLocation(LETTERBOX_PROGRAM, "scale");
    m_letterbox_color_uniform = glGetUniformLocation(LETTERBOX_PROGRAM, "color");
}

void GraphicsSystem::update(float dt)
{
}

void GraphicsSystem::render(void) const
{
    glClearColor(0.4, 0.4, 0.4, 1);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    m_active_scene->render();
    m_physics_debug->batchDrawLines();
    // TODO: Aspect ratio here
    glm::vec2 bar = m_active_scene->getViewportRemainder();
    glm::vec2 size = m_active_scene->getViewportSize();
    glDisable(GL_DEPTH_TEST);
    if(bar.x != 0) {
        glUseProgram(LETTERBOX_PROGRAM);
        glm::vec3 pos1(-1 + (bar.x / 2) / size.x, 0.0f, 0.0f);
        glm::vec3 pos2(1 - ((bar.x / 2) / size.x), 0.0f, 0.0f);
        glm::vec3 scale(bar.x / size.x, 2.0f, 1.0f);
        glEnableVertexAttribArray(m_letterbox_translation_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
        glVertexAttribPointer(m_letterbox_vertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glUniform3fv(m_letterbox_translation_uniform, 1, &pos1[0]);
        glUniform3fv(m_letterbox_scale_uniform, 1, &scale[0]);
        glUniform4f(m_letterbox_color_uniform, 0, 0, 0, 1);
        checkGLError();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform3fv(m_letterbox_translation_uniform, 1, &pos2[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkGLError();
    } else if(bar.y != 0) {
        glUseProgram(LETTERBOX_PROGRAM);
        checkGLError();
        glm::vec3 pos1(0.0f, -1 + (bar.y / 2) / size.y, 0.0f);
        glm::vec3 pos2(0.0f, 1 - ((bar.y / 2) / size.y), 0.0f);
        glm::vec3 scale(2.0f, bar.y / size.y, 1.0f);
        glEnableVertexAttribArray(m_letterbox_translation_uniform);
        glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
        glVertexAttribPointer(m_letterbox_vertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glUniform3fv(m_letterbox_translation_uniform, 1, &pos1[0]);
        glUniform3fv(m_letterbox_scale_uniform, 1, &scale[0]);
        glUniform4f(m_letterbox_color_uniform, 0, 0, 0, 1);
        checkGLError();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform3fv(m_letterbox_translation_uniform, 1, &pos2[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkGLError();
    }
    glEnable(GL_DEPTH_TEST);

    glfwSwapBuffers(m_main_window);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void GraphicsSystem::cleanup(void)
{
    delete m_active_scene;
    WindowData* window_data = static_cast<WindowData*>(glfwGetWindowUserPointer(m_main_window));
    delete window_data;
    delete m_physics_debug;
    glfwDestroyWindow(m_main_window);
}

void GraphicsSystem::setTitle(const char* title)
{
    if(m_main_window)
        glfwSetWindowTitle(m_main_window, title);
    else {
        warn("Trying to set the title of a nonexistent window.");
    }
}

void GraphicsSystem::loadSceneFromLevel(const Level* level)
{
    //if(m_active_scene)
        //delete m_active_scene;
    //m_active_scene = new Scene(level->getSceneData());
}

void GraphicsSystem::updateViewportSize(int width, int height)
{
    m_active_scene->updateViewportSize(width, height);
}

glm::vec2 GraphicsSystem::getViewportSize() const
{
    return m_active_scene->getViewportSize();
}

glm::vec2 GraphicsSystem::getViewportOffset() const
{
    return m_active_scene->getViewportRemainder();
}

void closeCallback(GLFWwindow* win)
{
    g_game->quit();
}

void sizeCallback(GLFWwindow* win, int width, int height)
{
    glViewport(0, 0, width, height);
    GraphicsSystem* system = static_cast<WindowData*>(glfwGetWindowUserPointer(win))->gfx_data;
    system->updateViewportSize(width, height);
}
