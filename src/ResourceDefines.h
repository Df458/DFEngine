#ifndef RESOURCE_DEFINES_H
#define RESOURCE_DEFINES_H
#define DATA_PATH "data/"
#define ACTOR_DATA_PATH "data/actors/"
#define AUDIO_DATA_PATH "data/audio/"
#define FONT_DATA_PATH "data/fonts/"
#define LEVEL_DATA_PATH "data/levels/"
#define MATERIAL_DATA_PATH "data/models/"
#define MODEL_DATA_PATH "data/models/"
#define PHYSICS_MATERIAL_DATA_PATH "data/physics/"
#define SHADER_DATA_PATH "data/shaders/"
#define SCRIPT_DATA_PATH "data/scripts/"
#define TEXTURE_DATA_PATH "data/textures/"

#define ACTOR_SUFFIX ".actr"
#define AUDIO_SUFFIX ".wav"
#define AUDIO_STREAM_SUFFIX ".ogg"
#define FONT_SUFFIX ".ttf"
#define LEVEL_SUFFIX ".lev"
#define MATERIAL_SUFFIX ".mtl"
#define MODEL_SUFFIX ".obj"
#define PHYSICS_MATERIAL_SUFFIX ".pmt"
#define VERTEX_SHADER_SUFFIX ".vert"
#define FRAGMENT_SHADER_SUFFIX ".frag"
#define SCRIPT_SUFFIX ".lua"
#define TEXTURE_SUFFIX ".png"

static const char* WIREFRAME_VERTEX_SHADER[] =
{"# version 120\n"
"attribute vec3 vertex_pos;\n"
"attribute vec3 vertex_color;\n"
"varying vec3 color;"
"uniform mat4 view_projection;\n"
"void main() {\n"
"gl_Position = view_projection * vec4(vertex_pos, 1.0);\n"
"color = vertex_color;"
"}"};
static const char* WIREFRAME_FRAGMENT_SHADER[] =
{"# version 120\n"
"varying vec3 color;"
"void main() {\n"
"gl_FragColor = vec4(color, 1);"
"}"};
static const char* SPRITE_VERTEX_SHADER[] =
{"# version 120\n"
"attribute vec3 vertex_pos;\n"
"varying vec2 uv;\n"
"uniform vec3 right;\n"
"uniform vec3 up;\n"
"uniform vec3 position;\n"
"uniform vec2 dims;\n"
"uniform mat4 model_view_projection;\n"
"void main() {\n"
"uv = (vertex_pos.xy - vec2(0.5, 0.5)) + 1;\n"// TODO: Might just want to add uvs to the buffer, it's likely cheaper in the long run
"vec3 wvp = right * vertex_pos.x * dims.x + up * vertex_pos.y * dims.y;\n"
"gl_Position = model_view_projection * vec4(wvp, 1.0);\n"
"}"};
static const char* SPRITE_FRAGMENT_SHADER[] =
{"# version 120\n"
"varying vec2 uv;\n"
"uniform vec4 color;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
"gl_FragColor = color * texture2D(texture, uv);\n"
"if(gl_FragColor.a == 0)\n"
"discard;\n"
"}"};
static const char* PARTICLE_VERTEX_SHADER[] =
{"# version 120\n"
"attribute vec3 vertex_pos;\n"
"varying vec2 uv;\n"
"varying vec4 p_color;\n"
"uniform vec3 right;\n"
"uniform vec3 up;\n"
"attribute vec3 position;\n"
"attribute vec4 color;\n"
"uniform vec2 dims;\n"
"uniform mat4 model_view_projection;\n"
"void main() {\n"
"uv = (vertex_pos.xy - vec2(0.5, 0.5)) + 1;\n"
"p_color = color;"
"vec3 wvp = position + right * vertex_pos.x * dims.x + up * vertex_pos.y * dims.y;\n"
"gl_Position = model_view_projection * vec4(wvp, 1.0);\n"
"}"};
static const char* PARTICLE_FRAGMENT_SHADER[] =
{"# version 120\n"
"varying vec2 uv;\n"
"varying vec4 p_color;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
"gl_FragColor = p_color * texture2D(texture, uv);\n"
"if(gl_FragColor.a == 0)\n"
"discard;\n"
"}"};

static const char* TEXT_VERTEX_SHADER[] =
{"# version 120\n"
"attribute vec3 vertex_pos;\n"
"varying vec2 uv;\n"
"uniform mat4 model_view_projection;\n"
"void main() {\n"
"uv = (vertex_pos.xy + vec2(0.5, 0.5)) * -1 + 1;\n"// TODO: Might just want to add uvs to the buffer, it's likely cheaper in the long run
"gl_Position = model_view_projection * vec4(vertex_pos, 1.0);\n"
"}"};
static const char* TEXT_FRAGMENT_SHADER[] =
{"# version 120\n"
"varying vec2 uv;\n"
"uniform vec3 color;\n"
"uniform sampler2D texture;\n"
"void main() {\n"
"gl_FragColor = vec4(color, texture2D(texture, uv).r);\n"
"if(gl_FragColor.a <= 0.9)\n"
"discard;\n"
"}"};

static const float QUAD_BUFFER_DATA[] =
{
// Vertex positon/uv
0.5, -0.5, 0,
-0.5, -0.5, 0,
-0.5, 0.5, 0,
0.5, -0.5, 0,
-0.5, 0.5, 0,
0.5, 0.5, 0,
};

static const float BLANK_TEXTURE_BUFFER_DATA[] =
{
    1.0f, 1.0f, 1.0f, 1.0f,
};

#endif
