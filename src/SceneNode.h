#ifndef SCENE_NODE_H
#define SCENE_NODE_H
#include "Color.h"
#include "Model.h"
#include "RenderUtil.h"
#include "Transform.h"
#include "Util.h"
#include "XmlSerializable.h"
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <vector>

class IScene;
class IShader;
class IFont;

class ISceneNode : public IXmlSerializable
{
public:
    virtual ~ISceneNode(void) = 0;
    virtual void draw(IScene* scene, RenderPass pass) = 0;
    virtual void drawChildren(IScene* scene, RenderPass pass) = 0;
    virtual bool addChild(ISceneNode* child) = 0;
    virtual bool removeChild(ISceneNode* child) = 0;
    virtual bool hasChild(ISceneNode* child) const = 0;
    virtual bool getVisible(void) const = 0;
    virtual bool getRenders(void) const = 0;
    virtual void setRenders(bool visible) = 0;
    virtual void deleteChildren(void) = 0;
    virtual RenderPass getPass(void) const = 0;
    virtual long getActor(void) const = 0;
    virtual void setTransform(Transform* trans) = 0;
    virtual void setLocalTransform(Transform* trans) = 0;
    virtual Transform* getLocalTransform(void) const = 0;
    virtual ISceneNode* getParent(void) const = 0;
    virtual const luaL_Reg* getFuncs(void) const = 0;
    virtual const luaL_Reg* getAttrFuncs(void) const = 0;
protected:
    virtual void setParent(ISceneNode* parent) = 0;
};

int node_render(lua_State* state);

int model_model(lua_State* state);
int model_shader(lua_State* state);
int model_texture(lua_State* state);

int camera_sky(lua_State* state);

int billboard_color(lua_State* state);
int billboard_texture(lua_State* state);

int particle_color(lua_State* state);
int particle_spawning(lua_State* state);
int particle_count(lua_State* state);

int text_text(lua_State* state);
int text_color(lua_State* state);

const luaL_Reg node_default_funcs[] =
{
    {0, 0}
};

const luaL_Reg node_default_attr[] =
{
    {"render", node_render},
    {0, 0}
};

const luaL_Reg node_camera_attr[] =
{
    {"sky_color", camera_sky},
    {0, 0}
};

const luaL_Reg node_model_attr[] =
{
    {"render", node_render},
    {"model", model_model},
    {"shader", model_shader},
    {"texture", model_texture},
    {0, 0}
};

const luaL_Reg node_billboard_attr[] =
{
    {"render", node_render},
    {"color", billboard_color},
    {"texture", billboard_texture},
    {0, 0}
};

const luaL_Reg node_particle_attr[] =
{
    {"render", node_render},
    {"spawning", particle_spawning},
    {"color", particle_color},
    {"particle_count", particle_count},
    {0, 0}
};

const luaL_Reg node_text_attr[] =
{
    {"render", node_render},
    {"text", text_text},
    {"color", text_color},
    {0, 0}
};

inline ISceneNode::~ISceneNode(void) {}

class SceneNode : public ISceneNode
{
public:
    SceneNode(void);
    ~SceneNode(void);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual void drawChildren(IScene* scene, RenderPass pass);
    virtual bool addChild(ISceneNode* child);
    virtual bool removeChild(ISceneNode* child);
    virtual bool hasChild(ISceneNode* child) const;
    virtual bool getVisible(void) const;
    virtual bool fromXml(rapidxml::xml_node<>* node);
    virtual bool getRenders(void) const { return m_renders; }
    virtual void setRenders(bool visible) { m_renders = visible; }
    virtual void deleteChildren(void);
    virtual RenderPass getPass(void) const;
    virtual long getActor(void) const;
    virtual ISceneNode* getParent(void) const { return u_parent; }
    virtual const luaL_Reg* getFuncs(void) const { return u_funcs; }
    virtual const luaL_Reg* getAttrFuncs(void) const { return u_attr_funcs; }
    void setTransform(Transform* trans) final;
    void setLocalTransform(Transform* trans) final { m_local_transform = trans; }
    Transform* getLocalTransform(void) const { return m_local_transform; }
protected:
    void setParent(ISceneNode* parent) final;
    glm::mat4 m_final_transform;
    Transform* m_local_transform = 0;
    Transform* u_transform_source = 0;
    RenderPass m_render_pass;
    bool m_renders = true;
    const luaL_Reg* u_funcs = node_default_funcs;
    const luaL_Reg* u_attr_funcs = node_default_attr;
private:
    std::vector<ISceneNode*> u_children;
    ISceneNode* u_parent = nullptr;
    long m_actor_id = -1;
};

class UpdatingSceneNode : public SceneNode
{
public:
    virtual void update(float delta_time) = 0;
};

class ModelSceneNode : public SceneNode
{
public:
    ModelSceneNode(void);
    ModelSceneNode(IModel* model, IShader* shader, Texture* texture = 0, RenderPass pass = RenderPass::DYNAMIC_PASS);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual bool getVisible(void);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    virtual const IModel* getModel(void) { return u_model; }
    virtual const IShader* getShader(void) { return u_shader; }
    virtual const Texture* getTexture(void) { return u_texture; }
    virtual void setModel(IModel* model) { u_model = model; }
    virtual void setShader(IShader* shader) { u_shader = shader; }
    virtual void setTexture(Texture* texture) { u_texture = texture; }
protected:
    IModel* u_model = nullptr;
    IShader* u_shader = nullptr;
    Texture* u_texture = 0;
};

class CameraSceneNode : public SceneNode
{
public:
    CameraSceneNode(void);
    CameraSceneNode(rapidxml::xml_node<>* node);
    inline const glm::mat4 getProjectionMatrix(void) const { return m_projection;}
    inline const glm::mat4 getViewMatrix(void) const { return m_view; }
    bool fromXml(rapidxml::xml_node<>* node);
    void reProject(float width, float height);
    void lookAt(glm::vec3 target);
    void setActive(bool active) { m_active = active; }
    bool getActive(void) { return m_active; }
    bool getOrtho(void) const { return m_ortho; }
    void setSkyColor(RGBColor color) { m_sky_color = color; }
    virtual const RGBColor& getSkyColor(void) const { return m_sky_color; }
    glm::vec2 getAspectDifference(void) const;
private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
    glm::mat4 m_offset;
    bool m_ortho = false;
    float m_fov;
    float m_near = 0.1;
    float m_far = 1000;
    bool m_active = false;
    RGBColor m_sky_color;
    glm::vec2 m_desired_dimensions = {1, 1};
    float m_aspect_ratio = 1;
    float m_desired_aspect_ratio = 1;
};

class LightSceneNode : public SceneNode
{
public:
    LightSceneNode(void);
    LightSceneNode(rapidxml::xml_node<>* node);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    bool  getAffectsDiffuse(void) const;
    bool  getAffectsSpecular(void) const;
    const RGBColor& getColor(void) const;
    float getStrength(void) const;
    void setAffectsDiffuse(bool affects_diffuse);
    void setAffectsSpecular(bool affects_specular);
    void setColor(const RGBColor& color);
    void setStrength(float strength);
protected:
    RGBColor m_color = RGBColor(1.0f, 1.0f, 1.0f);
    glm::vec3 m_direction = { 0, 1, 0 };
    bool m_diffuse = true;
    bool m_specular = true;
    float m_strength = 1.0f;
    GLuint u_program = 0;
    GLuint m_vertex_attrib = 0;
    GLuint m_color_uniform = 0;
    GLuint m_direction_uniform = 0;
    GLuint m_color_t_uniform = 0;
    GLuint m_normal_t_uniform = 0;
    GLuint m_position_t_uniform = 0;
    GLuint m_eye_position_uniform = 0;
};

class BillboardSceneNode : public SceneNode
{
public:
    BillboardSceneNode();
    BillboardSceneNode(Texture* texture, RGBAColor color = RGBAColor(Color::White, 1.0f), RenderPass pass = RenderPass::UI_PASS);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    void setColor(RGBAColor color) { m_color = color; }
    RGBAColor getColor(void) { return m_color; }
    virtual const Texture* getTexture(void) { return u_texture; }
    void setTexture(Texture* texture) { u_texture = texture; }
    virtual bool getVisible(void);
protected:
    GLuint m_vertex_position_attrib = 0;
    GLuint m_texture_uniform = 0;
    GLuint m_color_uniform = 0;
    GLuint m_direction_uniform = 0;
    GLuint m_transform_uniform = 0;
    GLuint m_up_uniform = 0;
    GLuint m_right_uniform = 0;
    GLuint m_dims_uniform = 0;

    RGBAColor m_color = RGBAColor(Color::White, 1.0f);
    Texture* u_texture = 0;
};

struct Particle
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec2 life = { 0, 0 };
    glm::vec4 color = { 1, 1, 1, 1 };
    float cam_distance = -1;
    bool operator<(Particle& rval) { return this->cam_distance > rval.cam_distance; }
};

class ParticleSceneNode : public UpdatingSceneNode
{
public:
    ParticleSceneNode();
    ParticleSceneNode(Texture* texture, RGBAColor color, float rate, float life = 4, glm::vec2 dims = {1, 1}, bool burst = false, RenderPass pass = RenderPass::UI_PASS);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    virtual bool getVisible(void);
    virtual void update(float delta_time);
    virtual void createParticle(void);
    bool getSpawning(void) { return m_spawning; }
    void setSpawning(bool spawning) { m_spawning = spawning; }
    RGBAColor getStartingColor(void) const { return m_starting_color; }
    void setStartingColor(RGBAColor color) { m_starting_color = color; }
    unsigned getParticleCount(void) { return m_particle_count; }
protected:
    GLuint m_vertex_position_attrib = 0;
    GLuint m_texture_uniform = 0;
    GLuint m_color_attrib = 0;
    GLuint m_transform_uniform = 0;
    GLuint m_up_uniform = 0;
    GLuint m_right_uniform = 0;
    GLuint m_dims_uniform = 0;
    GLuint m_position_attrib = 0;
    GLuint m_particle_attrib = 0;

    GLuint m_particle_buffer = 0;
    GLuint m_particle_buffer_p = 0;
    GLuint m_particle_buffer_c = 0;
    Texture* u_texture = 0;
    Particle m_particles[MAX_PARTICLES];
    Particle m_particle_template;
    glm::vec3 m_particle_pos[MAX_PARTICLES];
    glm::vec4 m_particle_color[MAX_PARTICLES];
    glm::vec3 m_last_cam;
    unsigned m_particle_count = 0;
    unsigned m_last = 0;
    glm::vec2 m_dims = { 1, 1 };
    bool m_spawning = true;
    bool m_burst = true;
    float m_rate = 100;
    float m_starting_life = 4;
    RGBAColor m_starting_color = { 1, 1, 1, 1 };
};

class TextSceneNode : public SceneNode
{
public:
    TextSceneNode();
    TextSceneNode(IFont* font, const char* text, RenderPass pass = RenderPass::UI_PASS);
    virtual void draw(IScene* scene, RenderPass pass);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    virtual bool getVisible(void);
    void setColor(RGBColor color) { m_color = color; }
    RGBColor getColor(void) { return m_color; }
    void setText(const char* text) { m_text = text; }
    std::string getText(void) { return m_text; }
protected:
    std::string m_text = "";
    IFont* u_font = NULL;
    RGBColor m_color = Color::White;
    float m_size = 16;
};

#endif
