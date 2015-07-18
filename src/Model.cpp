#include "Model.h"
#include "Util.h"

#include <cstdio>
#include <cstring>
#include <glm/vec2.hpp>

Model::Model(char* model_data)
{
    std::vector<glm::vec3> temp_vertex_data;
    std::vector<unsigned int> temp_index_data;
    std::vector<glm::vec3> temp_normal_data;
    std::vector<unsigned int> temp_normal_index_data;
    std::vector<glm::vec2> temp_uv_data;
    std::vector<unsigned int> temp_uv_index_data;
    char* position = strchr(model_data, '\n');
    while(position && position + 1) {
        switch(position[1]) {
            case '#': break; // Ignore comments

            case 'f': {
                unsigned int index_1;
                unsigned int index_2;
                unsigned int index_3;
                bool has_uvs = false;
                bool has_normals = false;
                // We need to determine the format that the face is in,
                // which is a gigantic pain in the rear.
                //
                // This is the best method I know.
                unsigned int i = 3;
                while(position[i] != ' ' && position[i] !='\n' && position[i] != 0) {
                    if(position[i] == '/') {
                        if(has_uvs || position[i+1] == '/') {
                            has_normals = true;
                            break;
                        } else {
                            has_uvs = true;
                        }
                    }
                    ++i;
                }

                // Note that tris are assumed. This *is* a game engine, after all.
                if(has_uvs) {
                    unsigned int uv_1;
                    unsigned int uv_2;
                    unsigned int uv_3;
                    if(has_normals) {
                        unsigned int normal_1;
                        unsigned int normal_2;
                        unsigned int normal_3;
                        sscanf(position, "\nf %u/%u/%u %u/%u/%u %u/%u/%u", &index_1, &uv_1, &normal_1, &index_2, &uv_2, &normal_2, &index_3, &uv_3, &normal_3);
                        temp_normal_index_data.push_back(normal_1 - 1);
                        temp_normal_index_data.push_back(normal_2 - 1);
                        temp_normal_index_data.push_back(normal_3 - 1);
                    } else {
                        sscanf(position, "\nf %u/%u %u/%u %u/%u", &index_1, &uv_1, &index_2, &uv_2, &index_3, &uv_3);
                    }
                    temp_uv_index_data.push_back(uv_1 - 1);
                    temp_uv_index_data.push_back(uv_2 - 1);
                    temp_uv_index_data.push_back(uv_3 - 1);
                } else if(has_normals) {
                    unsigned int normal_1;
                    unsigned int normal_2;
                    unsigned int normal_3;
                    sscanf(position, "\nf %u//%u %u//%u %u//%u", &index_1, &normal_1, &index_2, &normal_2, &index_3, &normal_3);
                    temp_normal_index_data.push_back(normal_1 - 1);
                    temp_normal_index_data.push_back(normal_2 - 1);
                    temp_normal_index_data.push_back(normal_3 - 1);
                } else
                    sscanf(position, "\nf %u %u %u", &index_1, &index_2, &index_3);
                temp_index_data.push_back(index_1 - 1);
                temp_index_data.push_back(index_2 - 1);
                temp_index_data.push_back(index_3 - 1);
                } break;
            case 'm': // mtllib
            case 'u': // usemtl
                warn("Material support in model files is unimplemented.");
                break;
            case 'o':
                warn("Object support in model files is unimplemented.");
                break;
            case 's':
                warn("Smoothing toggle support in model files is unimplemented.");
                break;
            case 'v':
                if(position + 2) {
                    switch(position[2]) {
                        case ' ': {
                            glm::vec3 vertex;
                            int arg_count = sscanf(position, "\nv %f %f %f", &vertex.x, &vertex.y, &vertex.z);
                            if(arg_count < 3) {
                                warn("Wrong number of arguments to vertex, expected >= 3.");
                                break;
                            }

                            temp_vertex_data.push_back(vertex);
                        } break;
                        case 'n': {
                            m_has_normals = true;
                            glm::vec3 normal;
                            int arg_count = sscanf(position, "\nvn %f %f %f", &normal.x, &normal.y, &normal.z);
                            if(arg_count < 3) {
                                warn("Wrong number of arguments to normal, expected 3.");
                                break;
                            }

                            temp_normal_data.push_back(normal);
                        } break;
                        case 'p':
                            warn("Parameter space vertex support in model files is unimplemented.");
                        break;
                        case 't': {
                            m_has_uvs = true;
                            glm::vec2 uv;
                            int arg_count = sscanf(position, "\nvt %f %f", &uv.x, &uv.y);
                            if(arg_count < 2) {
                                warn("Wrong number of arguments to uv coordinates, expected >= 2.");
                                break;
                            }

                            temp_uv_data.push_back(uv);
                            } break;
                        default:
                            warn("Unknown character encountered in model file.");
                    }
                } else {
                    warn("Model file is malformed or corrupt.");
                }
                break;
            default:
                warn("Unknown character encountered in model file.");
        }
        position = strchr(position + 1, '\n');
    }

    if(m_has_uvs) {
        m_vertex_data = new glm::vec3[temp_uv_index_data.size()];
        m_index_data = new unsigned int[temp_uv_index_data.size()];
        m_normal_data = new glm::vec3[temp_uv_index_data.size()];
        m_uv_data = new glm::vec2[temp_uv_data.size()];
        m_index_count = temp_uv_index_data.size();

        std::copy(temp_uv_data.begin(), temp_uv_data.end(), m_uv_data);
        for(unsigned int i = 0; i < temp_uv_index_data.size(); ++i) {
            m_vertex_data[temp_uv_index_data[i]] = temp_vertex_data[temp_index_data[i]];
            m_index_data[i] = temp_uv_index_data[i];
            if(m_has_normals)
                m_normal_data[temp_uv_index_data[i]] = temp_normal_data[temp_normal_index_data[i]];
        }
    } else {
        m_vertex_data = new glm::vec3[temp_vertex_data.size()];
        m_index_data = new unsigned int[temp_index_data.size()];
        if(m_has_normals)
            m_normal_data = new glm::vec3[temp_index_data.size()];
        m_index_count = temp_index_data.size();

        std::copy(temp_vertex_data.begin(), temp_vertex_data.end(), m_vertex_data);
        for(unsigned int i = 0; i < temp_index_data.size(); ++i) {
            m_index_data[i] = temp_index_data[i];
            if(m_has_normals)
                m_normal_data[m_index_data[i]] = temp_normal_data[temp_normal_index_data[i]];
        }
    }

    glGenBuffers(1, &m_vertices);
    glGenBuffers(1, &m_indices);
    glGenBuffers(1, &m_normals);
    glGenBuffers(1, &m_uvs);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_index_count * sizeof(glm::vec3), m_vertex_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_indices);
	glBufferData(GL_ARRAY_BUFFER, m_index_count * sizeof(GLuint), m_index_data, GL_STATIC_DRAW);

    if(m_has_normals) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normals);
        glBufferData(GL_ARRAY_BUFFER, m_index_count * sizeof(glm::vec3), m_normal_data, GL_STATIC_DRAW);
    }

    if(m_has_uvs && m_uv_data) {
        glBindBuffer(GL_ARRAY_BUFFER, m_uvs);
        glBufferData(GL_ARRAY_BUFFER, temp_uv_index_data.size() * sizeof(glm::vec2), m_uv_data, GL_STATIC_DRAW);
    }
}

Model::Model(void)
{
    m_vertex_data = new glm::vec3[4] {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    m_index_data = new GLuint[6] {0, 1, 2, 0, 2, 3};
    m_normal_data = new glm::vec3[4] {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
    m_has_normals = true;
    m_index_count = 6;

    glGenBuffers(1, &m_vertices);
    glGenBuffers(1, &m_indices);
    glGenBuffers(1, &m_normals);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertices);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), m_vertex_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_indices);
	glBufferData(GL_ARRAY_BUFFER, m_index_count * sizeof(GLuint), m_index_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_normals);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), m_normal_data, GL_STATIC_DRAW);
}

Model::~Model(void)
{
}

void Model::cleanup(void) const
{
    if(m_has_uvs && m_uv_data)
        delete[] m_uv_data;
    if(m_has_normals && m_normal_data)
        delete[] m_normal_data;
    if(m_index_data)
        delete[] m_index_data;
    if(m_vertex_data)
        delete[] m_vertex_data;

    glDeleteBuffers(1, &m_vertices);
    glDeleteBuffers(1, &m_indices);
    glDeleteBuffers(1, &m_normals);
    glDeleteBuffers(1, &m_uvs);
}

GLuint Model::getIndexCount(void) const
{
    return m_index_count;
}

GLuint Model::getVertices(void) const
{
    return m_vertices;
}

GLuint Model::getIndices(void) const
{
    return m_indices;
}

GLuint Model::getNormals(void) const
{
    return m_normals;
}

GLuint Model::getUVs(void) const
{
    return m_uvs;
}
