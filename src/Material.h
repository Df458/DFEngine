#ifndef MATERIAL_H
#define MATERIAL_H
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

struct Material
{
    Material(FILE* infile);
    glm::vec3 color_ambient = { 0.1f, 0.1f, 0.1f };
    GLuint texture_ambient = 0;
    glm::vec3 color_diffuse = { 0.7f, 0.7f, 0.7f };
    GLuint texture_diffuse = 0;
    glm::vec3 color_specular = { 1.0f, 1.0f, 1.0f };
    GLuint texture_specular = 0;
    float shininess = 10.0f;
    GLuint texture_shininess = 0;
    float opacity = 1.0f;
    GLuint normal_map = 0;
};

#endif
