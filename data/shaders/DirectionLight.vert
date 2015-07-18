#version 330
layout (location = 0) in vec3 vertex_pos;

out vec2 uv;

void main() {
	gl_Position = vec4(vertex_pos * -2, 1.0);
    uv = (vertex_pos.xy + vec2(0.5, 0.5)) * -1 + 1;
}
