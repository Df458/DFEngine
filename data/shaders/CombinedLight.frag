#version 330
in vec2 uv;

uniform sampler2D colortex;
uniform sampler2D diffusetex;
uniform sampler2D speculartex;

layout (location = 0) out vec4 frag_color;

void main() {
    vec4 texColor = texture2D(colortex, uv);
    vec4 texDiffuse = texture2D(diffusetex, uv);
    vec4 texSpecular = texture2D(speculartex, uv);
    frag_color = texDiffuse * texColor + texSpecular;
}
