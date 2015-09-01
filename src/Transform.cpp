#include "Transform.h"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

Transform::Transform()
{
    m_graphics_transform = glm::mat4(1.0f);
}

Transform::Transform(const Transform& transform)
{
    setWorldTransform(transform.m_graphics_transform);
}

Transform::Transform(const btTransform& transform)
{
    setWorldTransform(transform);
}

Transform::Transform(lua_State* state)
{
    setWorldTransform(state);
}

void Transform::getWorldTransform(btTransform& transform) const
{
    transform = m_physics_transform;
}

glm::mat4 Transform::getWorldTransform() const
{
    return m_graphics_transform;
}

void Transform::setWorldTransform(const btTransform& transform)
{
    m_physics_transform = transform;
    float mat[16];
    m_physics_transform.getOpenGLMatrix(mat);
    mat[15] = 1;
    m_graphics_transform = glm::scale(glm::make_mat4x4(mat), m_scaling);
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(m_graphics_transform, m_scaling, m_rotation, m_translation, skew, persp);
    m_translation_mat = glm::translate(glm::mat4(1), m_translation);
    m_rotation_mat = glm::mat4_cast(m_rotation);
}

void Transform::setWorldTransform(const glm::mat4& transform)
{
    m_graphics_transform = transform;
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(m_graphics_transform, m_scaling, m_rotation, m_translation, skew, persp);
    m_translation_mat = glm::translate(glm::mat4(1), m_translation);
    m_rotation_mat = glm::mat4_cast(m_rotation);
    m_scale_mat = glm::translate(glm::mat4(1), m_scaling);
}

void Transform::setWorldTransform(lua_State* state)
{
    lua_getfield(state, -1, "position");
    if(lua_isnil(state, -1))
        lua_pop(state, 1);
    else
    {
        lua_getfield(state, -1, "x");
        if(lua_isnumber(state, -1))
            m_translation.x = lua_tonumber(state, -1);
        lua_getfield(state, -2, "y");
        if(lua_isnumber(state, -1))
            m_translation.y = lua_tonumber(state, -1);
        lua_getfield(state, -3, "z");
        if(lua_isnumber(state, -1))
            m_translation.z = lua_tonumber(state, -1);
        m_translation_mat = glm::translate(glm::mat4(1), m_translation);
        lua_pop(state, 4);
    }
    
    glm::vec3 eul;
    lua_getfield(state, -1, "rotation");
    if(lua_isnil(state, -1))
        lua_pop(state, 1);
    else
    {
        lua_getfield(state, -1, "x");
        if(lua_isnumber(state, -1))
            eul.x = lua_tonumber(state, -1);
        lua_getfield(state, -2, "y");
        if(lua_isnumber(state, -1))
            eul.y = lua_tonumber(state, -1);
        lua_getfield(state, -3, "z");
        if(lua_isnumber(state, -1))
            eul.z = lua_tonumber(state, -1);
        m_rotation = glm::quat(eul);
        m_rotation_mat = glm::mat4_cast(m_rotation);
        lua_pop(state, 4);
    }

    lua_getfield(state, -1, "scale");
    if(lua_isnil(state, -1))
        lua_pop(state, 1);
    else
    {
        lua_getfield(state, -1, "x");
        if(!lua_isnil(state, -1))
            m_scaling.x = lua_tonumber(state, -1);
        else
            lua_pop(state, 1);
        lua_getfield(state, -2, "y");
        if(!lua_isnil(state, -1))
            m_scaling.y = lua_tonumber(state, -1);
        else
            lua_pop(state, 1);
        lua_getfield(state, -3, "z");
        if(!lua_isnil(state, -1))
            m_scaling.z = lua_tonumber(state, -1);
        else
            lua_pop(state, 1);
        lua_pop(state, 4);
    }
    m_scale_mat = glm::scale(glm::mat4(1), m_scaling);

    m_graphics_transform = m_scale_mat * m_rotation_mat * m_translation_mat;
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
}

void Transform::translate(const glm::vec3& translation, bool relative)
{
    if(relative) {
        m_graphics_transform[3] += glm::vec4(translation, 1.0f);
        m_translation += translation;
    } else {
        m_graphics_transform[3] = glm::vec4(translation, 1.0f);
        m_translation = translation;
    }
    m_translation_mat = glm::translate(glm::mat4(1), translation);
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
}

void Transform::translate(float x, float y, float z, bool relative)
{
    translate(glm::vec3(x, y, z), relative);
}

void Transform::rotate(const glm::quat& rotation, bool relative)
{
    if(relative) {
        m_rotation *= rotation;
        m_rotation_mat *= glm::mat4_cast(rotation);
        m_graphics_transform = glm::mat4_cast(rotation) * m_graphics_transform;
    } else {
        m_rotation = rotation;
        m_rotation_mat = glm::mat4_cast(rotation);
        m_graphics_transform = m_scale_mat * m_rotation_mat * m_translation_mat;
    }
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
}

void Transform::rotate(const glm::vec3& rotation, bool relative)
{
    rotate(glm::quat(rotation), relative);
}

void Transform::scale(const glm::vec3& scale, bool relative)
{
    if(relative) {
        m_scaling *= scale;
        m_scale_mat = glm::scale(m_scale_mat, scale);
        m_graphics_transform = glm::scale(glm::mat4(1), scale) * m_graphics_transform;
    } else {
        m_scaling = scale;
        m_scale_mat = glm::scale(glm::mat4(1), scale);
        m_graphics_transform = m_scale_mat * m_rotation_mat * m_translation_mat;
    }
}

void Transform::scale(float x, float y, float z, bool relative)
{
    scale(glm::vec3(x, y, z), relative);
}

void Transform::operator*=(Transform rval)
{
    m_graphics_transform *= rval.m_graphics_transform;
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
    glm::vec3 skew;
    glm::vec4 persp;
    glm::decompose(m_graphics_transform, m_scaling, m_rotation, m_translation, skew, persp);
    m_translation_mat = glm::translate(glm::mat4(1), m_translation);
    m_rotation_mat = glm::mat4_cast(m_rotation);
    m_scale_mat = glm::translate(glm::mat4(1), m_scaling);
}

int transform_index(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Transform is missing its instance!");
    Transform* transform = *static_cast<Transform**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    if(!strcmp(lua_tostring(state, 2), "position")) {
        lua_newtable(state);
        lua_pushnumber(state, transform->m_translation.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, transform->m_translation.y);
        lua_setfield(state, -2, "y");
        lua_pushnumber(state, transform->m_translation.z);
        lua_setfield(state, -2, "z");
    } else if(!strcmp(lua_tostring(state, 2), "rotation")) {
        glm::vec3 rot = transform->getERotation();
        lua_newtable(state);
        lua_pushnumber(state, rot.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, rot.y);
        lua_setfield(state, -2, "y");
        lua_pushnumber(state, rot.z);
        lua_setfield(state, -2, "z");
    } else if(!strcmp(lua_tostring(state, 2), "scale")) {
        lua_newtable(state);
        lua_pushnumber(state, transform->m_scaling.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, transform->m_scaling.y);
        lua_setfield(state, -2, "y");
        lua_pushnumber(state, transform->m_scaling.z);
        lua_setfield(state, -2, "z");
    } else
        return 0;
    return 1;
}

int transform_newindex(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Transform is missing its instance!");
    Transform* transform = *static_cast<Transform**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    if(!strcmp(lua_tostring(state, 2), "position")) {
        glm::vec3 v;
        lua_settop(state, 3);
        lua_getfield(state, -1, "x");
        if(lua_isnumber(state, -1))
            v.x = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "y");
        if(lua_isnumber(state, -1))
            v.y = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "z");
        if(lua_isnumber(state, -1))
            v.z = lua_tonumber(state, -1);
        lua_pop(state, 1);
        transform->translate(v, false);
    } else if(!strcmp(lua_tostring(state, 2), "rotation")) {
        glm::vec3 v;
        lua_settop(state, 3);
        lua_getfield(state, -1, "x");
        if(lua_isnumber(state, -1))
            v.x = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "y");
        if(lua_isnumber(state, -1))
            v.y = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "z");
        if(lua_isnumber(state, -1))
            v.z = lua_tonumber(state, -1);
        lua_pop(state, 1);
        transform->rotate(v, false);
    } else if(!strcmp(lua_tostring(state, 2), "scale")) {
        glm::vec3 v;
        lua_settop(state, 3);
        lua_getfield(state, -1, "x");
        if(lua_isnumber(state, -1))
            v.x = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "y");
        if(lua_isnumber(state, -1))
            v.y = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, -1, "z");
        if(lua_isnumber(state, -1))
            v.z = lua_tonumber(state, -1);
        lua_pop(state, 1);
        transform->scale(v, false);
    }
    return 0;
}
