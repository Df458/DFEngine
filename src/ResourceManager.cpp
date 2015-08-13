#include "Actor.h"
#include "Font.h"
#include "Level.h"
#include "Material.h"
#include "Model.h"
#include "RenderUtil.h"
#include "ResourceDefines.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Sound.h"
#include "Util.h"

#include <png.h>
#include <rapidxml.hpp>

GLuint WIREFRAME_PROGRAM;
GLuint SPRITE_PROGRAM;
GLuint PARTICLE_PROGRAM;
GLuint TEXT_PROGRAM;
GLuint QUAD_BUFFER;
GLuint BLANK_TEXTURE;

DFBaseResourceManager::~DFBaseResourceManager(void)
{
}

bool DFBaseResourceManager::initialize(void)
{
    // Initialize FreeType
    int error = FT_Init_FreeType(&m_font_library);
    if(error)
        return false;

    // Compile/Link the wireframe rendering debug shader
    GLuint vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    WIREFRAME_PROGRAM = glCreateProgram();
    SPRITE_PROGRAM = glCreateProgram();
    PARTICLE_PROGRAM = glCreateProgram();
    TEXT_PROGRAM = glCreateProgram();
    glShaderSource(vertex_shader, 1, WIREFRAME_VERTEX_SHADER, NULL);
    glShaderSource(fragment_shader, 1, WIREFRAME_FRAGMENT_SHADER, NULL);
    if(checkGLError())
        return false;

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(WIREFRAME_PROGRAM, vertex_shader);
    glAttachShader(WIREFRAME_PROGRAM, fragment_shader);
    glLinkProgram(WIREFRAME_PROGRAM);
    if(checkGLError())
        return false;
    
    glDetachShader(WIREFRAME_PROGRAM, vertex_shader);
    glDetachShader(WIREFRAME_PROGRAM, fragment_shader);

    glShaderSource(vertex_shader, 1, SPRITE_VERTEX_SHADER, NULL);
    glShaderSource(fragment_shader, 1, SPRITE_FRAGMENT_SHADER, NULL);
    GLsizei len;
    char log[1024];

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(SPRITE_PROGRAM, vertex_shader);
    glAttachShader(SPRITE_PROGRAM, fragment_shader);
    glLinkProgram(SPRITE_PROGRAM);
    if(checkGLError())
        return false;
    glGetShaderInfoLog(vertex_shader, 1024, &len, log);
    if(len)
        printf("Vertex Shader Log\n%s\n", log);
    glGetShaderInfoLog(fragment_shader, 1024, &len, log);
    if(len)
        printf("Fragment Shader Log\n%s\n", log);
    //exit(0);
    if(checkGLError())
        return false;
    
    glDetachShader(SPRITE_PROGRAM, vertex_shader);
    glDetachShader(SPRITE_PROGRAM, fragment_shader);

    glShaderSource(vertex_shader, 1, PARTICLE_VERTEX_SHADER, NULL);
    glShaderSource(fragment_shader, 1, PARTICLE_FRAGMENT_SHADER, NULL);

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(PARTICLE_PROGRAM, vertex_shader);
    glAttachShader(PARTICLE_PROGRAM, fragment_shader);
    glLinkProgram(PARTICLE_PROGRAM);
    if(checkGLError())
        return false;
    glGetShaderInfoLog(vertex_shader, 1024, &len, log);
    if(len)
        printf("Vertex Shader Log\n%s\n", log);
    glGetShaderInfoLog(fragment_shader, 1024, &len, log);
    if(len)
        printf("Fragment Shader Log\n%s\n", log);
    //exit(0);
    if(checkGLError())
        return false;
    
    glDetachShader(PARTICLE_PROGRAM, vertex_shader);
    glDetachShader(PARTICLE_PROGRAM, fragment_shader);

    glShaderSource(vertex_shader, 1, TEXT_VERTEX_SHADER, NULL);
    glShaderSource(fragment_shader, 1, TEXT_FRAGMENT_SHADER, NULL);

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(TEXT_PROGRAM, vertex_shader);
    glAttachShader(TEXT_PROGRAM, fragment_shader);
    glLinkProgram(TEXT_PROGRAM);
    if(checkGLError())
        return false;
    glGetShaderInfoLog(vertex_shader, 1024, &len, log);
    if(len)
        printf("Vertex Shader Log\n%s\n", log);
    glGetShaderInfoLog(fragment_shader, 1024, &len, log);
    if(len)
        printf("Fragment Shader Log\n%s\n", log);
    //exit(0);
    if(checkGLError())
        return false;
    
    glDetachShader(TEXT_PROGRAM, vertex_shader);
    glDetachShader(TEXT_PROGRAM, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if(checkGLError())
        return false;

    glGenBuffers(1, &QUAD_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, QUAD_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), QUAD_BUFFER_DATA, GL_STATIC_DRAW);

    glGenTextures(1, &BLANK_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, BLANK_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, BLANK_TEXTURE_BUFFER_DATA);
    if(checkGLError())
        return false;

    return true;
}

void DFBaseResourceManager::cleanup(void)
{
    for(auto i : m_actors) {
        i.second->cleanup();
        delete i.second;
    }

    for(auto i : m_fonts) {
        i.second->cleanup();
        delete i.second;
    }

    for(auto i : m_levels) {
        i.second->cleanup();
        delete i.second;
    }

    for(auto i : m_models) {
        i.second->cleanup();
        delete i.second;
    }

    for(auto i : m_shaders) {
        i.second->cleanup();
        delete i.second;
    }

    for(auto i : m_audio) {
        i.second->cleanup();
        delete i.second;
    }
    
    for(auto i : m_scripts) {
        delete[] i.second;
    }

    FT_Done_FreeType(m_font_library);
}

StaticActorConstructionData* DFBaseResourceManager::getActor(std::string id)
{
    id = ACTOR_DATA_PATH + id + ACTOR_SUFFIX;
    auto search = m_actors.find(id);
    if(search == m_actors.end()) {
        StaticActorConstructionData* load_result = _loadActor(id);
        if(!load_result) {
            error("Trying to load an actor that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

ISound* DFBaseResourceManager::getAudio(std::string id)
{
    id = AUDIO_DATA_PATH + id + AUDIO_SUFFIX;
    auto search = m_audio.find(id);
    if(search == m_audio.end()) {
        ISound* load_result = _loadAudio(id);
        if(!load_result) {
            error("Trying to load a sound that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

ISound* DFBaseResourceManager::getAudioStream(std::string id)
{
    id = AUDIO_DATA_PATH + id + AUDIO_STREAM_SUFFIX;
    auto search = m_audio.find(id);
    if(search == m_audio.end()) {
        ISound* load_result = _loadAudioStream(id);
        if(!load_result) {
            error("Trying to load a sound that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

IFont* DFBaseResourceManager::getFont(std::string id)
{
    id = FONT_DATA_PATH + id + FONT_SUFFIX;
    auto search = m_fonts.find(id);
    if(search == m_fonts.end()) {
        IFont* load_result = _loadFont(id);
        if(!load_result) {
            error("Trying to load a font that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

Level* DFBaseResourceManager::getLevel(std::string id)
{
    id = LEVEL_DATA_PATH + id + LEVEL_SUFFIX;
    auto search = m_levels.find(id);
    if(search == m_levels.end()) {
        Level* load_result = _loadLevel(id);
        if(!load_result) {
            error("Trying to load a level that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

IModel* DFBaseResourceManager::getModel(std::string id)
{
    id = MODEL_DATA_PATH + id + MODEL_SUFFIX;
    auto search = m_models.find(id);
    if(search == m_models.end()) {
        IModel* load_result = _loadModel(id);
        if(!load_result) {
            error("Trying to load a model that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

PhysicsMaterial DFBaseResourceManager::getPhysicsMaterial(std::string id)
{
    id = PHYSICS_MATERIAL_DATA_PATH + id + PHYSICS_MATERIAL_SUFFIX;
    auto search = m_physics_materials.find(id);
    if(search == m_physics_materials.end()) {
        PhysicsMaterial load_result = _loadPhysicsMaterial(id);
        return load_result;
    }

    return search->second;
}

GLuint DFBaseResourceManager::getProgram(std::string id)
{
    id = SHADER_DATA_PATH + id;
    auto search = m_programs.find(id);
    if(search == m_programs.end()) {
        GLuint load_result = _loadProgram(id);
        if(!load_result) {
            error("Trying to load a shader program that doesn't exist.");
            return 0;
        } else
            return load_result;
    }

    return search->second;
}

char* const DFBaseResourceManager::getScript(std::string id)
{
    id = SCRIPT_DATA_PATH + id;
    auto search = m_scripts.find(id);
    if(search == m_scripts.end()) {
        char* load_result = _loadScript(id);
        if(!load_result) {
            error("Trying to load a script that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

IShader* DFBaseResourceManager::getShader(std::string id)
{
    id = SHADER_DATA_PATH + id;
    auto search = m_shaders.find(id);
    if(search == m_shaders.end()) {
        IShader* load_result = _loadShader(id);
        if(!load_result) {
            error("Trying to load a shader that doesn't exist.");
            return NULL;
        } else
            return load_result;
    }

    return search->second;
}

Material* DFBaseResourceManager::getShaderMaterial(std::string id)
{
    id = MATERIAL_DATA_PATH + id + MATERIAL_SUFFIX;
    auto search = m_shader_materials.find(id);
    if(search == m_shader_materials.end()) {
        Material* load_result = _loadShaderMaterial(id);
        if(!load_result) {
            error("Trying to load a material that doesn't exist.");
            return 0;
        } else
            return load_result;
    }

    return search->second;
}

Texture* DFBaseResourceManager::getTexture(std::string id)
{
    id = TEXTURE_DATA_PATH + id + TEXTURE_SUFFIX;
    auto search = m_textures.find(id);
    if(search == m_textures.end()) {
        Texture* load_result = _loadTexture(id);
        if(!load_result) {
            error("Trying to load a texture that doesn't exist.");
            return 0;
        } else
            return load_result;
    }

    return search->second;
}

bool DFBaseResourceManager::loadActor(std::string id)
{
    StaticActorConstructionData* data = _loadActor(ACTOR_DATA_PATH + id + ACTOR_SUFFIX);
    if(!data) {
        warn("Trying to load an actor that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadAudio(std::string id)
{
    ISound* data = _loadAudio(AUDIO_DATA_PATH + id + AUDIO_SUFFIX);
    if(!data) {
        warn("Trying to load a sound that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadAudioStream(std::string id)
{
    ISound* data = _loadAudio(AUDIO_DATA_PATH + id + AUDIO_STREAM_SUFFIX);
    if(!data) {
        warn("Trying to load a sound that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadFont(std::string id)
{
    IFont* data = _loadFont(FONT_DATA_PATH + id + FONT_SUFFIX);
    if(!data) {
        warn("Trying to load a font that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadLevel(std::string id)
{
    Level* data = _loadLevel(LEVEL_DATA_PATH + id + LEVEL_SUFFIX);
    if(!data) {
        warn("Trying to load a level that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadModel(std::string id)
{
    IModel* data = _loadModel(MODEL_DATA_PATH + id + MODEL_SUFFIX);
    if(!data) {
        warn("Trying to load a model that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadPhysicsMaterial(std::string id)
{
    _loadPhysicsMaterial(PHYSICS_MATERIAL_DATA_PATH + id + PHYSICS_MATERIAL_SUFFIX);
    return true;
}

bool DFBaseResourceManager::loadProgram(std::string id)
{
    GLuint data = _loadProgram(SHADER_DATA_PATH + id);
    if(!data) {
        warn("Trying to load a shader program that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadScript(std::string id)
{
    char* data = _loadScript(SCRIPT_DATA_PATH + id);
    if(!data) {
        warn("Trying to load a script that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadShader(std::string id)
{
    IShader* data = _loadShader(SHADER_DATA_PATH + id);
    if(!data) {
        warn("Trying to load a shader that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadShaderMaterial(std::string id)
{
    Material* data = _loadShaderMaterial(MATERIAL_DATA_PATH + id + MATERIAL_SUFFIX);
    if(!data) {
        warn("Trying to load a material that doesn't exist.");
        return false;
    }
    return true;
}

bool DFBaseResourceManager::loadTexture(std::string id)
{
    Texture* data = _loadTexture(TEXTURE_DATA_PATH + id +TEXTURE_SUFFIX);
    if(!data) {
        warn("Trying to load a texture that doesn't exist.");
        return false;
    }
    return true;
}

StaticActorConstructionData* DFBaseResourceManager::_loadActor(std::string id)
{
    if(m_actors.find(id) != m_actors.end()) {
        warn("Trying to load an actor that already exists.");
        return m_actors[id];
    }
    char* filedata = loadFileContents(id);
    if(!filedata)
        return NULL;

    StaticActorConstructionData* actor_data = new StaticActorConstructionData(filedata);

    m_actors.emplace(id, actor_data);

    return actor_data;
}

ISound* DFBaseResourceManager::_loadAudio(std::string id)
{
    if(m_audio.find(id) != m_audio.end()) {
        warn("Trying to load a sound that already exists.");
        return m_audio[id];
    }

    ISound* snd = new SoundEffect();
    if(!snd->load(id.c_str(), SoundFile::WAV)) {
        delete snd;
        return NULL;
    }

    m_audio.emplace(id, snd);
    return snd;
}

ISound* DFBaseResourceManager::_loadAudioStream(std::string id)
{
    if(m_audio.find(id) != m_audio.end()) {
        warn("Trying to load a sound that already exists.");
        return m_audio[id];
    }

    ISound* snd = new SoundStream();
    if(!snd->load(id.c_str(), SoundFile::OGG)) {
        delete snd;
        return NULL;
    }

    m_audio.emplace(id, snd);
    return snd;
}

IFont* DFBaseResourceManager::_loadFont(std::string id)
{
    if(m_fonts.find(id) != m_fonts.end()) {
        warn("Trying to load an font that already exists.");
        return m_fonts[id];
    }

    Font* font = new Font(id.c_str(), m_font_library);
    if(font->failedLoad()) {
        delete font;
        return 0;
    }

    m_fonts.emplace(id, font);

    return font;
}

Level* DFBaseResourceManager::_loadLevel(std::string id)
{
    if(m_levels.find(id) != m_levels.end()) {
        warn("Trying to load an level that already exists.");
        return m_levels[id];
    }
    char* filedata = loadFileContents(id);
    if(!filedata)
        return NULL;

    Level* level_data = new Level(filedata);

    m_levels.emplace(id, level_data);

    return level_data;
}

IModel* DFBaseResourceManager::_loadModel(std::string id)
{
    if(m_models.find(id) != m_models.end()) {
        warn("Trying to load an model that already exists.");
        return m_models[id];
    }
    char* filedata = loadFileContents(id);
    if(!filedata)
        return NULL;

    IModel* model_data = new Model(filedata, id);
    delete[] filedata;

    m_models.emplace(id, model_data);

    return model_data;
}

PhysicsMaterial DFBaseResourceManager::_loadPhysicsMaterial(std::string id)
{
    if(m_physics_materials.find(id) != m_physics_materials.end()) {
        warn("Trying to load an physics material that already exists.");
        return m_physics_materials[id];
    }
    char* filedata = loadFileContents(id);
    if(!filedata)
        return NULL;

    rapidxml::xml_document<> doc;
    try {
        doc.parse<rapidxml::parse_validate_closing_tags>(filedata);
    } catch(rapidxml::parse_error e) {
        printf("%s: %s\n", e.what(), e.where<char>());
    }

    PhysicsMaterial mat(doc.first_node("material", 8));
    delete[] filedata;

    m_physics_materials.emplace(id, mat);

    return mat;
}

GLuint DFBaseResourceManager::_loadProgram(std::string id)
{
    if(m_programs.find(id) != m_programs.end()) {
        warn("Trying to load a shader program that already exists.");
        return m_programs[id];
    }
    char* vertex_filedata = loadFileContents(id + VERTEX_SHADER_SUFFIX);
    if(!vertex_filedata)
        return 0;

    char* fragment_filedata = loadFileContents(id + FRAGMENT_SHADER_SUFFIX);
    if(!fragment_filedata) {
        delete vertex_filedata;
        return 0;
    }

//:TODO: 01.02.15 20:30:21, df458
// Replace this with a more generic, data-driven setup
    GLuint vertex_shader, fragment_shader, program;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    glShaderSource(vertex_shader, 1, &vertex_filedata, NULL);
    glShaderSource(fragment_shader, 1, &fragment_filedata, NULL);
    checkGLError();

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    checkGLError();
    GLsizei len;
    char log[1024];
    glGetShaderInfoLog(vertex_shader, 1024, &len, log);
    if(len > 0)
        printf("Vertex Shader Log:\n%s\n", log);
    glGetShaderInfoLog(fragment_shader, 1024, &len, log);
    if(len > 0)
        printf("Fragment Shader Log:\n%s\n", log);
    glGetProgramInfoLog(program, 1024, &len, log);
    if(len > 0)
        printf("Program Log:\n%s\n", log);
    
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    delete[] vertex_filedata;
    delete[] fragment_filedata;
    checkGLError();

    m_programs.emplace(id, program);

    return program;
}

IShader* DFBaseResourceManager::_loadShader(std::string id)
{
    if(m_shaders.find(id) != m_shaders.end()) {
        warn("Trying to load a shader that already exists.");
        return m_shaders[id];
    }
    char* vertex_filedata = loadFileContents(id + VERTEX_SHADER_SUFFIX);
    if(!vertex_filedata)
        return NULL;

    char* fragment_filedata = loadFileContents(id + FRAGMENT_SHADER_SUFFIX);
    if(!fragment_filedata) {
        delete vertex_filedata;
        return NULL;
    }

//:TODO: 01.02.15 20:30:21, df458
// Replace this with a more generic, data-driven setup
    GLuint vertex_shader, fragment_shader, program;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    glShaderSource(vertex_shader, 1, &vertex_filedata, NULL);
    glShaderSource(fragment_shader, 1, &fragment_filedata, NULL);
    checkGLError();

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    checkGLError();
    GLsizei len;
    char log[1024];
    glGetShaderInfoLog(vertex_shader, 1024, &len, log);
    if(len > 0)
        printf("Vertex Shader Log:\n%s\n", log);
    glGetShaderInfoLog(fragment_shader, 1024, &len, log);
    if(len > 0)
        printf("Fragment Shader Log:\n%s\n", log);
    glGetProgramInfoLog(program, 1024, &len, log);
    if(len > 0)
        printf("Program Log:\n%s\n", log);
    
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    delete[] vertex_filedata;
    delete[] fragment_filedata;
    checkGLError();

    IShader* shader_data = new BasicShader(program, id);

    m_shaders.emplace(id, shader_data);
    m_programs.emplace(id, program);

    return shader_data;
}

Material* DFBaseResourceManager::_loadShaderMaterial(std::string id)
{
    if(m_shader_materials.find(id) != m_shader_materials.end()) {
        warn("Trying to load a material that already exists.");
        return m_shader_materials[id];
    }

    FILE* file = fopen(id.c_str(), "rb");
    if(!file) {
        warn("Could not open file."); // TODO: Make this more descriptive
        return 0;
    }
    Material* mat = new Material(file);
    m_shader_materials.emplace(id, mat);

    return mat;
}

char* DFBaseResourceManager::_loadScript(std::string id)
{
    if(m_scripts.find(id) != m_scripts.end()) {
        warn("Trying to load a script that already exists.");
        return m_scripts[id];
    }
    char* script = loadFileContents(id + SCRIPT_SUFFIX);
    if(!script) // TODO: Add a warning here
        return NULL;
    m_scripts.emplace(id, script);

    return script;
}

Texture* DFBaseResourceManager::_loadTexture(std::string id)
{
    if(m_textures.find(id) != m_textures.end()) {
        warn("Trying to load a texture that already exists.");
        return m_textures[id];
    }
	Texture* texture = new Texture();
    texture->name = id;
	glGenTextures(1, &texture->texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture->texture_handle);
	
	FILE* infile = fopen(id.c_str(), "rb");
	if(!infile) {
		warn("Could not open file.");
		return 0;
	}
	
	uint8_t header[8];
	png_structp pstruct;
	png_infop info_struct;
	png_byte* image_data;
	png_bytep* row_ptrs;
	
	fread(header, sizeof(uint8_t), 8, infile);
	if(png_sig_cmp(header, 0, 8)) {
        warn("File has an invalid header.");
		return 0;
	}
	pstruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!pstruct) {
		warn("Could not read structure of file.");
		return 0;
	}
	info_struct = png_create_info_struct(pstruct);
	if(!info_struct) {
		png_destroy_read_struct(&pstruct, NULL, NULL);
		warn("Could not create info_struct for file.");
		return 0;
	}
	if(setjmp(png_jmpbuf(pstruct))) {
		return 0;
	}
	
	png_init_io(pstruct, infile);
	png_set_sig_bytes(pstruct, 8);
	png_read_info(pstruct, info_struct);
	
	texture->texture_width = png_get_image_width(pstruct, info_struct);
	texture->texture_height = png_get_image_height(pstruct, info_struct);
    png_byte color_type = png_get_color_type(pstruct, info_struct);
    png_byte bit_depth = png_get_bit_depth(pstruct, info_struct);
    int number_of_passes = png_set_interlace_handling(pstruct);
    
	if(color_type == PNG_COLOR_TYPE_RGB) {
		png_set_filler(pstruct, 0xff, PNG_FILLER_AFTER);
	}
	
	png_read_update_info(pstruct, info_struct);
    
    if(setjmp(png_jmpbuf(pstruct))){
		return 0;
	}
	
	int rowbytes = png_get_rowbytes(pstruct, info_struct);
	//rowbytes += 3 - ((rowbytes-1) % 4);
	
	image_data = (png_byte*)malloc(rowbytes * texture->texture_height /** sizeof(png_byte)+15*/);
	row_ptrs = (png_bytep*)malloc(sizeof(png_bytep) * texture->texture_height);
	for(unsigned i = 0; i < texture->texture_height; i++){
		row_ptrs[texture->texture_height - 1 - i] = image_data + i * rowbytes;
	}
	
	png_read_image(pstruct, row_ptrs);
	
	/*if(png_get_color_type(pstruct, info_struct) != PNG_COLOR_TYPE_RGBA) {
		png_set_add_alpha(pstruct, 0xff, PNG_FILLER_AFTER);
		std::cerr << "Added Alpha channel\n";
	}*/
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->texture_width, texture->texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	
	png_destroy_read_struct(&pstruct, &info_struct, NULL);
	free(image_data);
	free(row_ptrs);
	fclose(infile);

    m_textures.emplace(id, texture);

    return texture;
}
