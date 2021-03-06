#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <btBulletDynamicsCommon.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

class Transform : public btMotionState
{
public:
    Transform();
    Transform(const Transform& transform);
    Transform(const btTransform& transform);
    Transform(lua_State* state);
    Transform(btTransform transform);
    void getWorldTransform(btTransform& transform) const;
    glm::mat4 getWorldTransform() const;
    void setWorldTransform(const btTransform& transform);
    void setWorldTransform(const glm::mat4& transform);
    void setWorldTransform(lua_State* state);
    void translate(const glm::vec3& translation, bool relative = false);
    void translate(float x, float y, float z, bool relative = false);
    void rotate(const glm::quat& rotation, bool relative = false);
    void rotate(const glm::vec3& rotation, bool relative = false);
    void scale(const glm::vec3& scale, bool relative = false);
    void scale(float x, float y, float z, bool relative = false);
    inline glm::vec3 getPosition(void) const { return m_translation; }
    glm::vec3 getERotation(void) const { return glm::eulerAngles(m_rotation); }
    glm::quat getQRotation(void) const { return m_rotation; }
    inline glm::vec3 getScaling(void) const { return m_scaling; }

    friend int transform_index(lua_State* state);
    friend int transform_newindex(lua_State* state);

    void operator*= (Transform rval);
private:
    btTransform m_physics_transform;
    glm::mat4 m_graphics_transform;

    glm::mat4 m_translation_mat;
    glm::mat4 m_rotation_mat;
    glm::mat4 m_scale_mat;

    glm::vec3 m_translation;
    glm::quat m_rotation;
    glm::vec3 m_scaling = { 1, 1, 1 };
};

int transform_index(lua_State* state);
int transform_newindex(lua_State* state);

const luaL_Reg transform_meta[] =
{
    {"__index", transform_index},
    {"__newindex", transform_newindex},
    {0, 0}
};

#endif
