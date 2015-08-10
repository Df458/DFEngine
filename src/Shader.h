#ifndef SHADER_H
#define SHADER_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>

class IScene;
class IModel;
struct Texture;

class IShader
{
public:
    virtual ~IShader(void) = 0;

    virtual void cleanup(void) = 0;
    virtual void prepareForRender(IScene* scene, IModel* model, glm::mat4 world_transform, Texture* texture = 0) = 0;
    virtual void postRender(void) = 0;
};
inline IShader::~IShader(void) {}

class BasicShader : public IShader
{
public:
    BasicShader(GLuint program);
    virtual ~BasicShader(void);

    virtual void cleanup(void);
    virtual void prepareForRender(IScene* scene, IModel* model, glm::mat4 world_matrix, Texture* texture = 0);
    virtual void postRender(void);
protected:
    GLuint m_program;
    GLuint m_vertex_position_attrib;
    GLuint m_vertex_uv_attrib;
    GLuint m_vertex_normal_attrib;
    GLuint m_w_uniform;
    GLuint m_vp_uniform;
    GLuint m_color_uniform;
    GLuint m_texture_uniform;
};

#endif
