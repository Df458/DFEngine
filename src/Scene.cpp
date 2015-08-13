#include "CGraphics.h"
#include "Event.h"
#include "Game.h"
#include "PhysicsSystem.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Util.h"

using namespace rapidxml;

glm::mat4 MatrixStack::getMatrix(void) const
{
    if(m_is_empty)
        return glm::mat4(1.0f);

    return m_result_stack.top();
}

void MatrixStack::push(glm::mat4 matrix)
{
// Multiply the matrix being added by the top of the stack.
// That way, geting the result is significantly faster.
    if(m_is_empty) {
        m_result_stack.push(matrix);
        m_is_empty = false;
    } else
        m_result_stack.push(m_result_stack.top() * matrix);
}

void MatrixStack::pop(void)
{
    if(!m_is_empty) {
        m_result_stack.pop();
        if(m_result_stack.empty())
            m_is_empty = true;
    }
}

Scene::Scene(void)
{
    m_view_dims = glm::vec2(800, 600);
    m_root_node = new SceneNode();
    glGenFramebuffers(1, &m_light_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_light_fbo);
    glGenTextures(4, m_light_textures);
    glGenTextures(1, &m_z_texture);
    glGenTextures(1, &m_final_texture);
    glGenTextures(1, &m_specular_texture);
    for(int i = 0; i < 4; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_light_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_light_textures[i], 0);
    }
    u_program = g_game->resources()->getProgram("CombinedLight");
    m_vertex_attrib = glGetAttribLocation(u_program, "vertex_pos");
    m_color_t_uniform = glGetUniformLocation(u_program, "colortex");
    m_diffuse_t_uniform = glGetUniformLocation(u_program, "diffusetex");
    m_specular_t_uniform = glGetUniformLocation(u_program, "speculartex");

    glBindTexture(GL_TEXTURE_2D, m_z_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_view_dims.x, m_view_dims.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_z_texture, 0);

    glBindTexture(GL_TEXTURE_2D, m_final_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_final_texture, 0);
    glBindTexture(GL_TEXTURE_2D, m_specular_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, m_specular_texture, 0);

    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, buffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Error code 0x%x: ", status);
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED: fprintf(stderr, "undefined.\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: fprintf(stderr, "incomplete attachment.\n"); break;
        }
        error("Failed to init framebuffer");
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

Scene::~Scene(void)
{
    glDeleteFramebuffers(1, &m_light_fbo);
    glDeleteTextures(4, m_light_textures);
    glDeleteTextures(1, &m_z_texture);
    delete m_root_node;
}

void Scene::render(void)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_light_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT4);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, buffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    checkGLError();

    for(int pass = FIRST_PASS; pass != LIGHTING_PASS; ++pass) {
        if(m_root_node != nullptr) {
            m_root_node->drawChildren(this, static_cast<RenderPass>(pass));
        }
        checkGLError();
    }
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    GLuint lbuf[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(2, lbuf);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.4, 0.4, 0.4, 1);
    if(m_active_camera)
        glClearColor(m_active_camera->getSkyColor().x, m_active_camera->getSkyColor().y, m_active_camera->getSkyColor().z, 1);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_light_fbo);
    //glClear(GL_COLOR_BUFFER_BIT);
    checkGLError();

    for (unsigned int i = 0 ; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_light_textures[i]);
    }
    m_root_node->drawChildren(this, LIGHTING_PASS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_light_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glUseProgram(u_program);
    glEnableVertexAttribArray(m_vertex_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
    glVertexAttribPointer(m_vertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    //glReadBuffer(GL_COLOR_ATTACHMENT0);

    //glReadBuffer(GL_COLOR_ATTACHMENT4);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_light_textures[0]);
    glUniform1i(m_color_t_uniform, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_final_texture);
    glUniform1i(m_diffuse_t_uniform, 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_specular_texture);
    glUniform1i(m_specular_t_uniform, 2);
    checkGLError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_light_fbo);
    checkGLError();

    glDisableVertexAttribArray(m_vertex_attrib);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    checkGLError();

    glBlitFramebuffer(0, 0, m_view_dims.x, m_view_dims.y, 0, 0, m_view_dims.x, m_view_dims.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(int pass = LIGHTING_PASS + 1; pass != LAST_PASS; ++pass) {
        if(m_root_node != nullptr) {
            m_root_node->drawChildren(this, static_cast<RenderPass>(pass));
        }
        checkGLError();
    }
    //checkGLError();

    for(int pass = LIGHTING_PASS + 1; pass != LAST_PASS; ++pass) {
        if(m_root_node != nullptr) {
            m_root_node->drawChildren(this, static_cast<RenderPass>(pass));
        }
    }
}

float Scene::getDPU(void) const
{
    if(m_active_camera->getOrtho())
        return g_game->physics()->getWorldScale();
    return m_dpu;
}

glm::mat4 const Scene::getActiveProjectionMatrix(void) const
{
    if(m_active_camera != nullptr)
        return m_active_camera->getProjectionMatrix();
    warn("Camera not found.");
    return glm::mat4(1.0f);
}

glm::mat4 const Scene::getActiveViewMatrix(void) const
{
    if(m_active_camera != nullptr)
        return m_active_camera->getViewMatrix();
    warn("Camera not found.");
    return glm::mat4(1.0f);
}

bool Scene::addChild(unsigned long id, ISceneNode* child)
{
    if(auto cam = dynamic_cast<CameraSceneNode*>(child)) {
        addCamera(id, cam);
    } else if(auto lit = dynamic_cast<LightSceneNode*>(child)) {
        addLight(id, lit);
    } 
    if(m_actor_nodes.find(id) == m_actor_nodes.end()) {
        m_actor_nodes[id] = std::set<ISceneNode*>();
        m_actor_nodes[id].insert(child);
    } else {
        m_actor_nodes[id].insert(child);
    }
    m_root_node->addChild(child);
    return true;
}

bool Scene::addLight(unsigned long id, LightSceneNode* light)
{
    return true;
}

bool Scene::addCamera(unsigned long id, CameraSceneNode* camera)
{
    if(!m_active_camera || camera->getActive()) {
        if(m_active_camera)
            m_active_camera->setActive(false);
        m_active_camera = camera;
    }
    return true;
}

bool Scene::removeChild(unsigned long id, ISceneNode* child)
{
    if(m_actor_nodes.find(id) != m_actor_nodes.end())
        //m_actor_nodes.erase(id);
        for(auto i : m_actor_nodes[id]) {
            m_actor_nodes[id].erase(i);
            return true;
        }
    warn("Trying to remove an unregistered SceneNode.");
    return false;
}

void Scene::pushMatrix(glm::mat4 matrix)
{
    m_matrices.push(matrix);
}

void Scene::popMatrix(void)
{
    m_matrices.pop();
}

glm::mat4 Scene::getMatrix(void) const
{
    return m_matrices.getMatrix();
}

void Scene::updateViewportSize(int width, int height)
{
    if(m_active_camera)
        m_active_camera->reProject(width, height);
    m_view_dims = glm::vec2(width, height);

    glDeleteFramebuffers(1, &m_light_fbo);
    glDeleteTextures(4, m_light_textures);
    glDeleteTextures(1, &m_z_texture);
    glDeleteTextures(1, &m_final_texture);
    glDeleteTextures(1, &m_specular_texture);
    glGenFramebuffers(1, &m_light_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_light_fbo);
    glGenTextures(4, m_light_textures);
    glGenTextures(1, &m_z_texture);
    glGenTextures(1, &m_final_texture);
    glGenTextures(1, &m_specular_texture);
    for(int i = 0; i < 4; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_light_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_light_textures[i], 0);
    }

    glBindTexture(GL_TEXTURE_2D, m_z_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_view_dims.x, m_view_dims.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_z_texture, 0);

    glBindTexture(GL_TEXTURE_2D, m_final_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_final_texture, 0);
    glBindTexture(GL_TEXTURE_2D, m_specular_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_dims.x, m_view_dims.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, m_specular_texture, 0);

    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, buffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Error code 0x%x: ", status);
        switch(status) {
            case GL_FRAMEBUFFER_UNDEFINED: fprintf(stderr, "undefined.\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: fprintf(stderr, "incomplete attachment.\n"); break;
        }
        error("Failed to init framebuffer");
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Scene::updateViewportSize()
{
    if(m_active_camera)
        m_active_camera->reProject(m_view_dims.x, m_view_dims.y);
}

void Scene::CGraphicsCreatedCallback(const IEvent& event)
{
    if(event.getEventType() != CGraphicsCreatedEvent::m_type) {
        warn("Scene expected CGraphicsCreatedEvent, but received something else.");
        return;
    }
    const CGraphicsCreatedEvent* e = dynamic_cast<const CGraphicsCreatedEvent*>(&event);
    addChild(e->getId(), e->getNode());
}

void Scene::actorRemovedCallback(const IEvent& event)
{
    if(event.getEventType() != ActorDestroyedEvent::m_type) {
        warn("Scene expected ActorDestroyedEvent, but received something else.");
        return;
    }
    const ActorDestroyedEvent* e = dynamic_cast<const ActorDestroyedEvent*>(&event);
    deleteRecursive(e->getId());
}

void Scene::deleteRecursive(unsigned long id)
{
    bool removed_something = false;
    ISceneNode* node;
    std::stack<ISceneNode*> nodes;
    auto it = m_actor_nodes.find(id);
    if(it != m_actor_nodes.end()) {
        for(auto i : it->second)
            nodes.push(i);
        it->second.clear();
    }

    if(nodes.empty()) {
        warn("Trying to delete a SceneNode that doesn't exist.");
    } else {
        while(!nodes.empty()) {
            ISceneNode* node = nodes.top();
            nodes.pop();
            node->deleteChildren();
            if(node->getParent())
                node->getParent()->removeChild(node);
            else
                m_root_node->removeChild(node);
        }
    }
}
