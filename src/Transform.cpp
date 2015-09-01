#include "Transform.h"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    lua_getfield(state, -1, "position");
    lua_getfield(state, -1, "x");
    if(lua_isnumber(state, -1))
        m_translation.x = lua_tonumber(state, -1);
    lua_getfield(state, -2, "y");
    if(lua_isnumber(state, -1))
        m_translation.y = lua_tonumber(state, -1);
    lua_getfield(state, -3, "z");
    if(lua_isnumber(state, -1))
        m_translation.z = lua_tonumber(state, -1);
    lua_pop(state, 4);
    
    // TODO: Rotations
    glm::vec3 eul;
    lua_getfield(state, -1, "rotation");
    lua_getfield(state, -1, "x");
    if(lua_isnumber(state, -1))
        eul.x = lua_tonumber(state, -1);
    lua_getfield(state, -2, "y");
    if(lua_isnumber(state, -1))
        eul.y = lua_tonumber(state, -1);
    lua_getfield(state, -3, "z");
    if(lua_isnumber(state, -1))
        eul.z = lua_tonumber(state, -1);
    lua_pop(state, 4);

    lua_getfield(state, -1, "scale");
    lua_getfield(state, -1, "x");
    if(lua_isnumber(state, -1))
        m_scaling.x = lua_tonumber(state, -1);
    lua_getfield(state, -2, "y");
    if(lua_isnumber(state, -1))
        m_scaling.y = lua_tonumber(state, -1);
    lua_getfield(state, -3, "z");
    if(lua_isnumber(state, -1))
        m_scaling.z = lua_tonumber(state, -1);
    lua_pop(state, 4);
}

void Transform::getWorldTransform(btTransform& transform) const
{
    //float mat[16];
    //m_physics_transform.getOpenGLMatrix(mat);
    //mat[15] = 1;
    //printf("Bullet getting: \n");
    //for(int i = 0; i < 16; ++i)
        //printf("%f ",mat[i]);
    //printf("\n");
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
    glm::decompose(m_graphics_transform, &m_translation, &m_rotation, &m_scaling);
}

void Transform::setWorldTransform(const glm::mat4& transform)
{
    m_graphics_transform = transform;
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
}

void Transform::translate(const glm::vec3& translation, bool relative)
{
    if(relative) {
        m_graphics_transform[3] += glm::vec4(translation, 1.0f);
    } else {
        m_graphics_transform[3] = glm::vec4(translation, 1.0f);
    }
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
    m_translation = translation;
}

void Transform::translate(float x, float y, float z, bool relative)
{
    translate(glm::vec3(x, y, z), relative);
}

void Transform::rotate(const glm::quat& rotation, bool relative)
{
    if(relative) {
        m_graphics_transform = glm::mat4_cast(rotation) * m_graphics_transform;
        m_rotation *= rotation;
    } else {
        warn("Absolute transform rotation is unimplimented!");
        m_rotation = rotation;
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
        m_graphics_transform = glm::scale(glm::mat4(1), scale) * m_graphics_transform;
        m_scaling *= scale;
    } else {
        warn("Absolute transform scaling is unimplimented!");
        m_scaling = scale;
    }
}

glm::vec3 Transform::getPosition(void) const
{
    glm::vec4 vec = m_graphics_transform[3];
    return glm::vec3(vec);
}

glm::vec3 Transform::getRotation(void) const
{
    // TODO: Implement this
    return glm::vec3(0, 0, 0);
}

void Transform::operator*=(Transform rval)
{
    m_graphics_transform *= rval.m_graphics_transform;
    m_physics_transform.setFromOpenGLMatrix(glm::value_ptr(m_graphics_transform));
}
