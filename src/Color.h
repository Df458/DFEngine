#ifndef COLOR_H
#define COLOR_H
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

typedef glm::vec3 RGBColor;
typedef glm::vec4 RGBAColor;

namespace Color {
    static const RGBColor Red(1.0f, 0.0f, 0.0f);
    static const RGBColor Green(0.0f, 1.0f, 0.0f);
    static const RGBColor Blue(0.0f, 0.0f, 1.0f);
    static const RGBColor Yellow(1.0f, 1.0f, 0.0f);
    static const RGBColor Magenta(1.0f, 0.0f, 1.0f);
    static const RGBColor Cyan(0.0f, 1.0f, 1.0f);
    static const RGBColor White(1.0f, 1.0f, 1.0f);
    static const RGBColor Black(0.0f, 0.0f, 1.0f);
    static const RGBColor Gray(0.5f, 0.5f, 0.5f);
    static const RGBColor LightGray(0.75f, 0.75f, 0.75f);
    static const RGBColor DarkGray(0.25f, 0.25f, 0.25f);
    static const RGBColor Orange(1.0f, 0.5f, 0.0f);

    static const RGBAColor Clear(0.0f, 0.0f, 0.0f, 0.0f);
}

#endif
