#ifndef MODEL_H
#define MODEL_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

class IModel
{
public:
    virtual ~IModel(void) = 0;

    virtual void cleanup(void) const = 0;
    virtual GLuint getIndexCount(void) const = 0;
    virtual GLuint getVertices(void) const = 0;
    virtual GLuint getIndices(void) const = 0;
    virtual GLuint getNormals(void) const = 0;
    virtual GLuint getUVs(void) const = 0;
};
inline IModel::~IModel(void) {}

class Model : public IModel
{
public:
    Model(char* model_data);
    Model(void);
    ~Model(void);

    virtual void cleanup(void) const;
    virtual GLuint getIndexCount(void) const;
    virtual GLuint getVertices(void) const;
    virtual GLuint getIndices(void) const;
    virtual GLuint getNormals(void) const;
    virtual GLuint getUVs(void) const;
protected:
    unsigned int m_index_count;
    GLuint m_vertices;
    GLuint m_indices;
    GLuint m_normals;
    GLuint m_uvs;
    glm::vec3* m_vertex_data = 0;
    unsigned int* m_index_data = 0;
    glm::vec3* m_normal_data = 0;
    glm::vec2* m_uv_data = 0;
    bool m_has_uvs = false;
    bool m_has_normals = false;
};

#endif
