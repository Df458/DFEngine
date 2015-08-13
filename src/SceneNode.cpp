#include "CGraphics.h"
#include "Game.h"
#include "ResourceManager.h"
#include "Font.h"
#include "PhysicsSystem.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Shader.h"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>

using namespace rapidxml;

SceneNode::SceneNode()
{
    m_local_transform = new Transform();
}

SceneNode::~SceneNode(void)
{
    delete m_local_transform;
}

void SceneNode::draw(IScene* scene, RenderPass pass)
{
    if(pass != m_render_pass || !m_renders)
        return;
}

void SceneNode::drawChildren(IScene* scene, RenderPass pass)
{
    if(!m_renders)
        return;

    scene->pushMatrix(m_final_transform);
    for(ISceneNode* node : u_children) {
        node->draw(scene, pass);
        node->drawChildren(scene, pass);
    }
    scene->popMatrix();
}

bool SceneNode::addChild(ISceneNode* child)
{
    if(child == nullptr) {
        warn("Trying to add NULL as a child of a SceneNode.");
        return false;
    }

    if(hasChild(child)) {
        warn("Trying to add a SceneNode to its existing parent.");
        return false;
    }
    u_children.push_back(child);
    if(auto ch = dynamic_cast<SceneNode*>(child))
        ch->setParent(nullptr);
    return true;
}

bool SceneNode::removeChild(ISceneNode* child)
{
    for(auto i = u_children.begin(); i != u_children.end(); ++i) {
        if(*i == child) {
            u_children.erase(i);
            if(auto ch = dynamic_cast<SceneNode*>(child))
                ch->setParent(nullptr);
            return true;
        }
    }

    warn("Trying to remove a child from the wrong parent.");
    return false;
}

bool SceneNode::hasChild(ISceneNode* child) const
{
    for(ISceneNode* node : u_children) {
        if(node == child)
            return true;
    }
    return false;
}

bool SceneNode::fromXml(rapidxml::xml_node<>* node)
{
    attr(node, "renders", &m_renders);

    if(xml_node<>* tr = node->first_node("translate", 9, false)) {
        glm::vec3 translation(0, 0, 0);
        attr(tr, "x", &translation.x);
        attr(tr, "y", &translation.y);
        attr(tr, "z", &translation.z);
        m_local_transform->translate(translation);
    } 

    if(xml_node<>* tr = node->first_node("rotate", 6, false)) {
        glm::vec3 rotation(0, 0, 0);
        attr(tr, "x", &rotation.x);
        attr(tr, "y", &rotation.y);
        attr(tr, "z", &rotation.z);
        m_local_transform->rotate(rotation);
    }
    return true;
}

void SceneNode::setParent(ISceneNode* parent)
{
    u_parent = parent;
}

void SceneNode::setTransform(Transform* trans)
{
    u_transform_source = trans;
    m_final_transform = u_transform_source->getWorldTransform();
}

bool SceneNode::getVisible(void) const
{
    return false;
}

void SceneNode::deleteChildren(void)
{
    while(u_children.size() > 0)
        removeChild(*u_children.begin());
}

RenderPass SceneNode::getPass(void) const
{
    return m_render_pass;
}

long SceneNode::getActor(void) const
{
    return m_actor_id;
}

ModelSceneNode::ModelSceneNode(void)
{
    u_attr_funcs = node_model_attr;
    u_model = g_game->resources()->getModel("default");
    u_shader = g_game->resources()->getShader("default");
    u_texture = g_game->resources()->getTexture("default");
}

ModelSceneNode::ModelSceneNode(IModel* model, IShader* shader, Texture* texture, RenderPass pass) : SceneNode()
{
    u_attr_funcs = node_model_attr;
    u_model = model;
    u_shader = shader;
    m_render_pass = pass;
    u_texture = texture;
}

void ModelSceneNode::draw(IScene* scene, RenderPass pass)
{
    if(pass != m_render_pass || !m_renders)
        return;
    m_final_transform = scene->getMatrix() * u_transform_source->getWorldTransform() * m_local_transform->getWorldTransform();
    u_shader->prepareForRender(scene, u_model, m_final_transform, u_texture);

    glDrawElements(GL_TRIANGLES, u_model->getIndexCount(), GL_UNSIGNED_INT, 0);
    checkGLError();

    u_shader->postRender();
}

bool ModelSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    SceneNode::fromXml(node);
    if(xml_attribute<>* model_att = node->first_attribute("mesh", 4, false))
        u_model = g_game->resources()->getModel(model_att->value());

    if(xml_attribute<>* shader_att = node->first_attribute("shader", 6, false))
        u_shader = g_game->resources()->getShader(shader_att->value());
    if(xml_attribute<>* tex_att = node->first_attribute("texture", 7, false))
        u_texture = g_game->resources()->getTexture(tex_att->value());
    return true;
}

bool ModelSceneNode::getVisible(void)
{
    warn("Calculating visibility is unimplemented.");
    return false;
}

CameraSceneNode::CameraSceneNode() : SceneNode()
{
}

CameraSceneNode::CameraSceneNode(xml_node<>* node)
{
    fromXml(node);
}

void CameraSceneNode::reProject(float width, float height)
{
    if(!m_ortho)
        m_projection = glm::perspective(m_fov, width / height, m_near, m_far);
    else
        m_projection = glm::ortho(0.0f, width * g_game->physics()->getWorldScale(), height * g_game->physics()->getWorldScale(), 0.0f, m_near, m_far);
}

bool CameraSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    if(xml_node<>* projection = node->first_node("projection", 10, false)) {
        bool perspective = true;
        attr(projection, "near", &m_near);
        attr(projection, "far", &m_far);
        if(xml_attribute<>* p_type = projection->first_attribute("type", 4, false)) {
            if(!strcmp(p_type->value(), "perspective")) {
                m_fov = glm::radians(90.0f);
                attr(projection, "fov", &m_fov);
                m_projection = glm::perspective(m_fov, 4.0f / 3.0f, m_near, m_far);
            } else if(!strcmp(p_type->value(), "ortho")) {
                float width = 1;
                float height = 1;
                attr(projection, "width", &width);
                attr(projection, "height", &height);
                m_projection = glm::ortho(0.0f, width, height, 0.0f, m_near, m_far);
                m_ortho = true;
            } else {
                warn("Invalid projection type specified.");
                m_projection = glm::mat4(1.0f);
            }
        } else {
            warn("No projection type specified.");
            m_projection = glm::mat4(1.0f);
        }
    } else {
        warn("No projection specified");
        m_projection = glm::mat4(1.0f);
    }

    if(xml_node<>* v_rotate = node->first_node("rotate", 6, false)) {
        glm::vec3 rotation;
        attr(v_rotate, "x", &rotation.x);
        attr(v_rotate, "y", &rotation.y);
        attr(v_rotate, "z", &rotation.z);
        m_final_transform = glm::mat4_cast(glm::quat(rotation));
    }

    if(xml_node<>* v_translate = node->first_node("translate", 9, false)) {
        glm::vec3 translation;

        attr(v_translate, "x", &translation.x);
        attr(v_translate, "y", &translation.y);
        attr(v_translate, "z", &translation.z);
        glm::translate(m_final_transform, translation);
    }

    if(xml_node<>* v_lookat = node->first_node("lookat", 6, false)) {
        glm::vec3 target;
        attr(v_lookat, "x", &target.x);
        attr(v_lookat, "y", &target.y);
        attr(v_lookat, "z", &target.z);
        m_view = glm::lookAt(glm::vec3(m_final_transform[3]), target, glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        m_view = glm::inverse(m_final_transform);
    }

    attr(node, "active", &m_active);
    return true;
}

void CameraSceneNode::lookAt(glm::vec3 target)
{
    glm::vec3 self = glm::vec3(m_final_transform[3]);
    if(u_transform_source)
        self = u_transform_source->getPosition();
    m_view = glm::lookAt(self, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

LightSceneNode::LightSceneNode(void) : SceneNode()
{
}

LightSceneNode::LightSceneNode(xml_node<>* node)
{
    fromXml(node);
//:TODO: 20.02.15 20:32:50, df458
// Add extra
    m_render_pass = LIGHTING_PASS;
    u_program = g_game->resources()->getProgram("DirectionLight");
    m_vertex_attrib = glGetAttribLocation(u_program, "vertex_pos");
    m_color_t_uniform = glGetUniformLocation(u_program, "colortex");
    m_normal_t_uniform = glGetUniformLocation(u_program, "normaltex");
    m_direction_uniform = glGetUniformLocation(u_program, "direction");
    m_position_t_uniform = glGetUniformLocation(u_program, "positiontex");
    m_eye_position_uniform = glGetUniformLocation(u_program, "eye_position");
    m_color_uniform = glGetUniformLocation(u_program, "color");
    checkGLError();
}

void LightSceneNode::draw(IScene* scene, RenderPass pass)
{
    if(pass != m_render_pass || !m_renders)
        return;
    glUseProgram(u_program);
    checkGLError();

    glm::mat4 view = glm::inverse(scene->getActiveViewMatrix());
    glUniform3f(m_eye_position_uniform, view[3][0], view[3][1], view[3][2]);
    glUniform3f(m_direction_uniform, m_direction.x, m_direction.y, m_direction.z);

    glUniform4f(m_color_uniform, m_color.x, m_color.y, m_color.z, 1.0f);
    checkGLError();
    glEnableVertexAttribArray(m_vertex_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
    glVertexAttribPointer(m_vertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(m_color_t_uniform, 0);
    glUniform1i(m_position_t_uniform, 1);
    glUniform1i(m_normal_t_uniform, 2);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGLError();

    glDisableVertexAttribArray(m_vertex_attrib);
}

bool LightSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    SceneNode::fromXml(node);

    if(xml_node<>* color = node->first_node("color", 5, false)) {
        attr(color, "r", &m_color.x);
        attr(color, "g", &m_color.y);
        attr(color, "b", &m_color.z);
    }
    if(xml_node<>* direction = node->first_node("direction", 9, false)) {
        if(xml_attribute<>* x = direction->first_attribute("x", 1, false))
            m_direction.x = atof(x->value());
        if(xml_attribute<>* y = direction->first_attribute("y", 1, false))
            m_direction.y = atof(y->value());
        if(xml_attribute<>* z = direction->first_attribute("z", 1, false))
            m_direction.z = atof(z->value());
        attr(direction, "x", &m_direction.x);
        attr(direction, "y", &m_direction.y);
        attr(direction, "z", &m_direction.z);
    }
    attr(node, "strength", &m_strength);
    attr(node, "affects_diffuse", &m_diffuse);
    attr(node, "affects_specular", &m_specular);

    return true;
}

bool LightSceneNode::getAffectsDiffuse(void) const
{
    return m_diffuse;
}

bool LightSceneNode::getAffectsSpecular(void) const
{
    return m_specular;
}

const RGBColor& LightSceneNode::getColor(void) const
{
    return m_color;
}

float LightSceneNode::getStrength(void) const
{
    return m_strength;
}

void LightSceneNode::setAffectsDiffuse(bool affects_diffuse)
{
    m_diffuse = affects_diffuse;
}

void LightSceneNode::setAffectsSpecular(bool affects_specular)
{
    m_specular = affects_specular;
}

void LightSceneNode::setColor(const RGBColor& color)
{
    m_color = color;
}

void LightSceneNode::setStrength(float strength)
{
    m_strength = strength;
}

BillboardSceneNode::BillboardSceneNode()
{
    u_attr_funcs = node_billboard_attr;
    m_render_pass = RenderPass::UI_PASS;
    m_vertex_position_attrib = glGetAttribLocation(SPRITE_PROGRAM, "vertex_pos");
    m_texture_uniform = glGetUniformLocation(SPRITE_PROGRAM, "texture");
    m_color_uniform = glGetUniformLocation(SPRITE_PROGRAM, "color");
    m_transform_uniform = glGetUniformLocation(SPRITE_PROGRAM, "model_view_projection");
    m_up_uniform = glGetUniformLocation(SPRITE_PROGRAM, "up");
    m_right_uniform = glGetUniformLocation(SPRITE_PROGRAM, "right");
    m_dims_uniform = glGetUniformLocation(SPRITE_PROGRAM, "dims");
    checkGLError();
}

BillboardSceneNode::BillboardSceneNode(Texture* texture, RGBAColor color, RenderPass pass) : SceneNode()
{
    u_attr_funcs = node_billboard_attr;
    m_render_pass = pass;
    u_texture = texture;
    m_vertex_position_attrib = glGetAttribLocation(SPRITE_PROGRAM, "vertex_pos");
    m_texture_uniform = glGetUniformLocation(SPRITE_PROGRAM, "texture");
    m_color_uniform = glGetUniformLocation(SPRITE_PROGRAM, "color");
    m_transform_uniform = glGetUniformLocation(SPRITE_PROGRAM, "model_view_projection");
    m_up_uniform = glGetUniformLocation(SPRITE_PROGRAM, "up");
    m_right_uniform = glGetUniformLocation(SPRITE_PROGRAM, "right");
    m_dims_uniform = glGetUniformLocation(SPRITE_PROGRAM, "dims");
    checkGLError();
    m_color = color;
}

void BillboardSceneNode::draw(IScene* scene, RenderPass pass)
{
    if(pass != m_render_pass || !m_renders)
        return;
    m_final_transform = scene->getMatrix() * u_transform_source->getWorldTransform() * m_local_transform->getWorldTransform();

    glUseProgram(SPRITE_PROGRAM);
    checkGLError();

    glm::mat4 view = scene->getActiveViewMatrix();
    glm::mat4 world = scene->getMatrix() * m_final_transform;
    glm::vec3 pos;
    glm::vec3 scale;
    glm::quat rot;
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(world, scale, rot, pos, skew, persp);
    glm::mat4 transform_matrix = scene->getActiveProjectionMatrix() * view * glm::translate(glm::mat4(1), pos) * glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, -glm::roll(rot)))) * glm::scale(glm::mat4(1), scale);
    glUniformMatrix4fv(m_transform_uniform, 1, GL_FALSE, &transform_matrix[0][0]);
    glUniform3f(m_up_uniform, view[0][1], view[1][1], view[2][1]);
    checkGLError();
    glUniform3f(m_right_uniform, view[0][0], view[1][0], view[2][0]);
    glUniform2f(m_dims_uniform, (float)u_texture->texture_width * scene->getDPU(), (float)u_texture->texture_height * scene->getDPU());
    glUniform4f(m_color_uniform, m_color.x, m_color.y, m_color.z, m_color.w);
    glEnableVertexAttribArray(m_vertex_position_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
    glVertexAttribPointer(m_vertex_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, u_texture->texture_handle);
    glUniform1i(m_texture_uniform, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGLError();

    glDisableVertexAttribArray(m_vertex_position_attrib);
}

bool BillboardSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    SceneNode::fromXml(node);

    if(xml_attribute<>* tex_att = node->first_attribute("id", 2, false))
        u_texture = g_game->resources()->getTexture(tex_att->value());
    if(xml_node<>* ncolor = node->first_node("color", 5, false)) {
        attr(ncolor, "r", &m_color.x);
        attr(ncolor, "g", &m_color.y);
        attr(ncolor, "b", &m_color.z);
        attr(ncolor, "a", &m_color.w);
    }
    return true;
}

bool BillboardSceneNode::getVisible(void)
{
    warn("Calculating visibility is unimplemented.");
    return false;
}

ParticleSceneNode::ParticleSceneNode()
{
    u_attr_funcs = node_particle_attr;
    m_render_pass = RenderPass::UI_PASS;
    m_vertex_position_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "vertex_pos");
    m_texture_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "texture");
    m_color_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "color");
    m_transform_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "model_view_projection");
    m_position_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "position");
    m_up_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "up");
    m_right_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "right");
    m_dims_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "dims");
    glGenBuffers(1, &m_particle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), NULL, GL_STREAM_DRAW);
    glGenBuffers(1, &m_particle_buffer_p);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_p);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
    glGenBuffers(1, &m_particle_buffer_c);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_c);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
    checkGLError();
}

ParticleSceneNode::ParticleSceneNode(Texture* texture, RGBAColor color, float rate, float life, glm::vec2 dims, bool burst, RenderPass pass) : UpdatingSceneNode()
{
    u_attr_funcs = node_particle_attr;

    m_render_pass = pass;
    u_texture = texture;
    m_vertex_position_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "vertex_pos");
    m_texture_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "texture");
    m_color_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "color");
    m_transform_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "model_view_projection");
    m_position_attrib = glGetAttribLocation(PARTICLE_PROGRAM, "position");
    m_up_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "up");
    m_right_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "right");
    m_dims_uniform = glGetUniformLocation(PARTICLE_PROGRAM, "dims");

    glGenBuffers(1, &m_particle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), NULL, GL_STREAM_DRAW);
    glGenBuffers(1, &m_particle_buffer_p);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_p);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
    glGenBuffers(1, &m_particle_buffer_c);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_c);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
    checkGLError();
    m_dims = dims;
    m_burst = burst;
    m_spawning = true;
    m_starting_color = color;
    m_starting_life = life;
    m_rate = rate;
}

void ParticleSceneNode::draw(IScene* scene, RenderPass pass)
{
    if(pass != m_render_pass || !m_renders)
        return;
    glDisable(GL_DEPTH_TEST);
    glm::mat4 view = scene->getActiveViewMatrix();
    m_last_cam = glm::vec3(glm::inverse(view)[3]);

    //std::sort(&m_particles[0], &m_particles[MAX_PARTICLES]);

    m_final_transform = scene->getMatrix() * u_transform_source->getWorldTransform() * m_local_transform->getWorldTransform();

    glUseProgram(PARTICLE_PROGRAM);
    checkGLError();

    glm::mat4 transform_matrix = scene->getActiveProjectionMatrix() * view;
    glUniformMatrix4fv(m_transform_uniform, 1, GL_FALSE, &transform_matrix[0][0]);
    glUniform3f(m_up_uniform, view[0][1], view[1][1], view[2][1]);
    glUniform3f(m_right_uniform, view[0][0], view[1][0], view[2][0]);
    glUniform2f(m_dims_uniform, m_dims.x, m_dims.y);
    glEnableVertexAttribArray(m_vertex_position_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
    glVertexAttribPointer(m_vertex_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    for(unsigned i = 0; i < m_particle_count; ++i) {
        m_particle_pos[i] = m_particles[i].position;
        m_particle_color[i] = m_particles[i].color;
    }

    glEnableVertexAttribArray(m_color_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_p);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particle_count * sizeof(glm::vec4), m_particle_color);
    glVertexAttribPointer(m_color_attrib, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(m_position_attrib);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer_c);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particle_count * sizeof(glm::vec3), m_particle_pos);
    glVertexAttribPointer(m_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    checkGLError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, u_texture->texture_handle);
    glUniform1i(m_texture_uniform, 0);
    checkGLError();

    glVertexAttribDivisor(m_vertex_position_attrib, 0);
    glVertexAttribDivisor(m_color_attrib, 1);
    glVertexAttribDivisor(m_position_attrib, 1);
    checkGLError();

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_particle_count);
    checkGLError();

    glDisableVertexAttribArray(m_vertex_position_attrib);
    glDisableVertexAttribArray(m_color_attrib);
    glDisableVertexAttribArray(m_position_attrib);
    glVertexAttribDivisor(m_vertex_position_attrib, 0);
    glVertexAttribDivisor(m_color_attrib, 0);
    glVertexAttribDivisor(m_position_attrib, 0);
    glEnable(GL_DEPTH_TEST);
}

bool ParticleSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    UpdatingSceneNode::fromXml(node);

    if(xml_attribute<>* tex_att = node->first_attribute("id", 2, false))
        u_texture = g_game->resources()->getTexture(tex_att->value());
    attr(node, "width", &m_dims.x);
    attr(node, "height", &m_dims.y);
    attr(node, "rate", &m_rate);
    attr(node, "life", &m_starting_life);
    attr(node, "burst", &m_burst);
    attr(node, "spawning", &m_spawning);
    RGBAColor color(Color::White, 1.0f);
    if(xml_node<>* ncolor = node->first_node("color", 5, false)) {
        attr(ncolor, "r", &m_starting_color.x);
        attr(ncolor, "g", &m_starting_color.y);
        attr(ncolor, "b", &m_starting_color.z);
        attr(ncolor, "a", &m_starting_color.w);
    }
    return true;
}

void ParticleSceneNode::update(float delta_time)
{
    if(m_spawning) {
        unsigned new_count = delta_time * m_rate;
        if(new_count > m_rate * 1.5 || m_burst)
            new_count = m_rate;
        for(unsigned i = 0; i < new_count; ++i)
            createParticle();
        if(m_burst)
            m_spawning = false;
    }
    m_particle_count = 0;
    for(unsigned i = 0; i < MAX_PARTICLES; ++i) {
        Particle& particle = m_particles[i];

        if(particle.life.x > 0.0f) {
            particle.life.x -= delta_time;
            particle.velocity += particle.acceleration * delta_time;
            particle.position += particle.velocity * delta_time;
            particle.cam_distance = glm::length2(particle.position - m_last_cam);
            particle.color.w = particle.life.x / particle.life.y * m_starting_color.w;
            ++m_particle_count;
        } else {
            particle.cam_distance = -1.0f;
        }
    }
}

void ParticleSceneNode::createParticle(void)
{
    for(unsigned i = m_last; i < MAX_PARTICLES; ++i) {
        if(m_particles[i].life.x <= 0) {
            m_last = i;
            m_particles[i].life = {m_starting_life, m_starting_life};
            m_particles[i].velocity.x = (rand() % 100) / 20.0f - 2.5;
            m_particles[i].velocity.y = (rand() % 100) / 20.0f - 2.5;
            m_particles[i].velocity.z = 0;
            //m_particles[i].acceleration = m_particles[i].velocity * 0.7f;
            m_particles[i].color = m_starting_color;//{(rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f}
            //m_particles[i].position = glm::vec3(m_final_transform[3]);
            m_particles[i].position = glm::vec3(u_transform_source->getWorldTransform()[3]);
            m_particles[i].cam_distance = glm::length2(m_particles[i].position - m_last_cam);
            return;
        }
    }
    for(unsigned i = 0; i < m_last; ++i) {
        if(m_particles[i].life.x <= 0) {
            m_last = i;
            m_particles[i].life = {m_starting_life, m_starting_life};
            m_particles[i].velocity.x = (rand() % 100) / 500.0f - 0.1;
            m_particles[i].velocity.y = (rand() % 100) / 500.0f - 0.1;
            m_particles[i].velocity.z = (rand() % 100) / 500.0f - 0.1;
            m_particles[i].acceleration = m_particles[i].velocity * 0.1f;
            m_particles[i].color = m_starting_color;//{(rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f};
            //m_particles[i].position = glm::vec3(m_final_transform[3]);
            m_particles[i].position = glm::vec3(u_transform_source->getWorldTransform()[3]);
            m_particles[i].cam_distance = glm::length2(m_particles[i].position - m_last_cam);
            return;
        }
    }
}

bool ParticleSceneNode::getVisible(void)
{
    warn("Calculating visibility is unimplemented.");
    return false;
}

TextSceneNode::TextSceneNode()
{
    m_render_pass = RenderPass::UI_PASS;
    u_attr_funcs = node_text_attr;
}

TextSceneNode::TextSceneNode(IFont* font, const char* text, RenderPass pass) : SceneNode()
{
    u_attr_funcs = node_text_attr;

    u_font = font;
    m_render_pass = pass;
    m_text = text;
}

void TextSceneNode::draw(IScene* scene, RenderPass pass)
{
    m_final_transform = scene->getMatrix() * u_transform_source->getWorldTransform() * m_local_transform->getWorldTransform();
    u_font->draw(scene, m_text.c_str(), m_final_transform);
}

bool TextSceneNode::fromXml(rapidxml::xml_node<>* node)
{
    SceneNode::fromXml(node);

    if(xml_attribute<>* text_att = node->first_attribute("value", 5, false))
        m_text = text_att->value();
    if(xml_attribute<>* fn_att = node->first_attribute("font", 4, false))
        u_font = g_game->resources()->getFont(fn_att->value());
    return true;
}

bool TextSceneNode::getVisible(void)
{
    warn("Calculating visibility is unimplemented.");
    return false;
}

int node_render(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access render state, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    SceneNode* node = (SceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setRenders(lua_toboolean(state, 2));
        return 0;
    } else
        lua_pushboolean(state, node->getRenders());
    return 1;
}

int model_model(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access a model, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    ModelSceneNode* node = (ModelSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setModel(g_game->resources()->getModel(lua_tostring(state, 2))); 
        return 0;
    }
    lua_pushstring(state, node->getModel()->getName().c_str());
    return 1;
}

int model_shader(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access a model, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    ModelSceneNode* node = (ModelSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setShader(g_game->resources()->getShader(lua_tostring(state, 2)));
        return 0;
    }
    lua_pushstring(state, node->getShader()->getName().c_str());
    return 1;
}

int model_texture(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access a model, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    ModelSceneNode* node = (ModelSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setTexture(g_game->resources()->getTexture(lua_tostring(state, 2)));
        return 0;
    }
    lua_pushstring(state, node->getTexture()->name.c_str());
    return 1;
}

int billboard_texture(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access a billboard, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    BillboardSceneNode* node = (BillboardSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setTexture(g_game->resources()->getTexture(lua_tostring(state, 2)));
        return 0;
    }
    lua_pushstring(state, node->getTexture()->name.c_str());
    return 1;
}

int billboard_color(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access a billboard, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    BillboardSceneNode* node = (BillboardSceneNode*)gfx->getNode();
    RGBAColor color;
    if(lua_gettop(state) > 1) {
        lua_getfield(state, 2, "r");
        color.x = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "g");
        color.y = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "b");
        color.z = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "a");
        color.w = lua_tonumber(state, -1);
        lua_pop(state, 1);
        node->setColor(color);
        return 0;
    }
    color = node->getColor();
    lua_pushstring(state, node->getTexture()->name.c_str());
    lua_newtable(state);
    lua_pushnumber(state, color.x);
    lua_setfield(state, -2, "r");
    lua_pushnumber(state, color.y);
    lua_setfield(state, -2, "g");
    lua_pushnumber(state, color.z);
    lua_setfield(state, -2, "b");
    lua_pushnumber(state, color.w);
    lua_setfield(state, -2, "a");
    return 1;
}

int particle_spawning(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access render state, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    ParticleSceneNode* node = (ParticleSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setSpawning(lua_toboolean(state, 2));
        return 0;
    } else
        lua_pushboolean(state, node->getSpawning());
    return 1;
}

int particle_count(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access render state, but the Graphics Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    ParticleSceneNode* node = (ParticleSceneNode*)gfx->getNode();
    lua_pushinteger(state, node->getParticleCount());
    return 1;
}

int text_text(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access text, but the Text Component is missing its instance!");
    CGraphics* gfx = *static_cast<CGraphics**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    TextSceneNode* node = (TextSceneNode*)gfx->getNode();
    if(lua_gettop(state) > 1) {
            node->setText(lua_tostring(state, 2));
        return 0;
    } else
        lua_pushstring(state, node->getText().c_str());
    return 1;
}
