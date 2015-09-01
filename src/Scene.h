#ifndef SCENE_H
#define SCENE_H

#include "RenderUtil.h"
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <map>
#include <rapidxml.hpp>
#include <stack>
#include <set>

class ISceneNode;
class CameraSceneNode;
class LightSceneNode;
class IEvent;

class MatrixStack
{
public:
    glm::mat4 getMatrix(void) const;
    void push(glm::mat4 matrix);
    void pop(void);
private:
    std::stack<glm::mat4> m_result_stack;
    bool m_is_empty = true;
};

class IScene
{
public:
    virtual ~IScene(void) = 0;
    virtual void render(void) = 0;
    virtual const glm::mat4 getActiveProjectionMatrix(void) const = 0;
    virtual const glm::mat4 getActiveViewMatrix(void) const = 0;
    virtual bool addChild(unsigned long id, ISceneNode* child) = 0;
    virtual bool removeChild(unsigned long id, ISceneNode* child) = 0;
    virtual void pushMatrix(glm::mat4 matrix) = 0;
    virtual void popMatrix(void) = 0;
    virtual glm::mat4 getMatrix(void) const = 0;
    virtual void updateViewportSize(int width, int height) = 0;
    virtual void updateViewportSize() = 0;
    virtual glm::vec2 getViewportSize(void) const = 0;
    virtual GLuint getLightTexture(int id) const = 0;
    virtual float getDPU(void) const = 0;
    virtual glm::vec2 getViewportRemainder(void) const = 0;
    
    virtual void CGraphicsCreatedCallback(const IEvent& event) = 0;
    virtual void actorRemovedCallback(const IEvent& event) = 0;
protected:
    virtual void deleteRecursive(unsigned long id) = 0;
};

inline IScene::~IScene(void) {}

class Scene : public IScene
{
public:
    Scene(void);
    //Scene(const Scene* scene);
    //Scene(rapidxml::xml_node<>* const node);
    virtual ~Scene(void);
    virtual void render(void);
    virtual const glm::mat4 getActiveProjectionMatrix(void) const;
    virtual const glm::mat4 getActiveViewMatrix(void) const;
    virtual bool addChild(unsigned long id, ISceneNode* child);
    virtual bool addLight(unsigned long id, LightSceneNode* light);
    virtual bool addCamera(unsigned long id, CameraSceneNode* camera);
    virtual bool removeChild(unsigned long id, ISceneNode* child);
    virtual void pushMatrix(glm::mat4 matrix);
    virtual void popMatrix(void);
    virtual glm::mat4 getMatrix(void) const;
    virtual void updateViewportSize(int width, int height);
    virtual void updateViewportSize();
    virtual glm::vec2 getViewportSize(void) const { return m_view_dims; }
    virtual GLuint getLightTexture(int id) const { return m_light_textures[id]; }
    virtual float getDPU(void) const;
    virtual glm::vec2 getViewportRemainder(void) const;

    virtual void CGraphicsCreatedCallback(const IEvent& event);
    virtual void actorRemovedCallback(const IEvent& event);
private:
    virtual void deleteRecursive(unsigned long id);

    ISceneNode* m_root_node;
    CameraSceneNode* m_active_camera = nullptr;
    std::map<unsigned long, LightSceneNode*> m_light_nodes;
    std::map<unsigned long, CameraSceneNode*> m_camera_nodes;
    std::map<unsigned long, std::set<ISceneNode*>> m_actor_nodes;
    MatrixStack m_matrices;
    glm::vec2 m_view_dims;
    GLuint m_light_textures[4] = { 0 };
    GLuint m_light_fbo = 0;
    GLuint m_z_texture = 0;
    GLuint m_final_texture = 0;
    GLuint m_specular_texture = 0;

    GLuint u_program = 0;
    GLuint m_vertex_attrib = 0;
    GLuint m_color_t_uniform = 0;
    GLuint m_diffuse_t_uniform = 0;
    GLuint m_specular_t_uniform = 0;

    float m_dpu = 100;
};

#endif
