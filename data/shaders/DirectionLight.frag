#version 330
in vec2 uv;

uniform vec3 eye_position; // TODO: Bind this
uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 direction;
uniform sampler2D colortex;
uniform sampler2D normaltex;
uniform sampler2D positiontex;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 spec_color;

void main() {
    vec4 texColor = texture2D(colortex, uv);
    vec4 texNormal = texture2D(normaltex, uv);
    vec4 texPosition = texture2D(positiontex, uv);
    vec3 reverse_direction = direction * -1;
    float diffuse_power = max(dot(normalize(texNormal).xyz, reverse_direction), 0);

    vec4 eye_diff = vec4(eye_position, 1) - texPosition;
    vec3 to_eye = normalize(eye_diff).xyz;
    vec3 reflected = normalize(reflect(reverse_direction, normalize(texNormal).xyz));
    float specular_power = max(dot(to_eye, reflected), 0);
    vec4 ambient = vec4(texColor.xyz * 0.5, texColor.a);
    vec4 diffuse = texColor * color * diffuse_power;
    vec4 specular = texColor * color * specular_power;
    frag_color = diffuse + ambient;
    frag_color.a = 1;
    spec_color = specular;
}
