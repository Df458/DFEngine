#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "PhysicsMaterial.h"

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <unordered_map>
#include FT_FREETYPE_H

class StaticActorConstructionData;
class IFont;
class Level;
class IModel;
class IShader;
class ISound;
struct Material;
struct Texture;

class ResourceManager
{
public:
    virtual ~ResourceManager(void) = 0;
    virtual bool initialize(void) = 0;
    virtual void cleanup(void) = 0;

    virtual StaticActorConstructionData* getActor(std::string id) = 0;
    virtual ISound* getAudio(std::string id) = 0;
    virtual ISound* getAudioStream(std::string id) = 0;
    virtual IFont* getFont(std::string id) = 0;
    virtual Level* getLevel(std::string id) = 0;
    virtual IModel* getModel(std::string id) = 0;
    virtual PhysicsMaterial getPhysicsMaterial(std::string id) = 0;
    virtual GLuint getProgram(std::string id) = 0;
    virtual char* const getScript(std::string id) = 0;
    virtual IShader* getShader(std::string id) = 0;
    virtual Material* getShaderMaterial(std::string id) = 0;
    virtual Texture* getTexture(std::string id) = 0;
    virtual bool loadActor(std::string id) = 0;
    virtual bool loadAudio(std::string id) = 0;
    virtual bool loadAudioStream(std::string id) = 0;
    virtual bool loadFont(std::string id) = 0;
    virtual bool loadLevel(std::string id) = 0;
    virtual bool loadModel(std::string id) = 0;
    virtual bool loadPhysicsMaterial(std::string id) = 0;
    virtual bool loadProgram(std::string id) = 0;
    virtual bool loadScript(std::string id) = 0;
    virtual bool loadShader(std::string id) = 0;
    virtual bool loadShaderMaterial(std::string id) = 0;
    virtual bool loadTexture(std::string id) = 0;
};
inline ResourceManager::~ResourceManager() {}

class DFBaseResourceManager : public ResourceManager
{
public:
    ~DFBaseResourceManager(void);
    virtual bool initialize(void);
    virtual void cleanup(void);

    virtual StaticActorConstructionData* getActor(std::string id);
    virtual ISound* getAudio(std::string id);
    virtual ISound* getAudioStream(std::string id);
    virtual IFont* getFont(std::string id);
    virtual Level* getLevel(std::string id);
    virtual IModel* getModel(std::string id);
    virtual PhysicsMaterial getPhysicsMaterial(std::string id);
    virtual GLuint getProgram(std::string id);
    virtual char* const getScript(std::string id);
    virtual IShader* getShader(std::string id);
    virtual Material* getShaderMaterial(std::string id);
    virtual Texture* getTexture(std::string id);
    virtual bool loadActor(std::string id);
    virtual bool loadAudio(std::string id);
    virtual bool loadAudioStream(std::string id);
    virtual bool loadFont(std::string id);
    virtual bool loadLevel(std::string id);
    virtual bool loadModel(std::string id);
    virtual bool loadPhysicsMaterial(std::string id);
    virtual bool loadProgram(std::string id);
    virtual bool loadScript(std::string id);
    virtual bool loadShader(std::string id);
    virtual bool loadShaderMaterial(std::string id);
    virtual bool loadTexture(std::string id);

protected:
    std::unordered_map<std::string, StaticActorConstructionData*> m_actors;
    std::unordered_map<std::string, ISound*> m_audio;
    std::unordered_map<std::string, IFont*> m_fonts;
    std::unordered_map<std::string, Level*> m_levels;
    std::unordered_map<std::string, IModel*> m_models;
    std::unordered_map<std::string, PhysicsMaterial> m_physics_materials;
    std::unordered_map<std::string, GLuint> m_programs;
    std::unordered_map<std::string, IShader*> m_shaders;
    std::unordered_map<std::string, Material*> m_shader_materials;
    std::unordered_map<std::string, char*> m_scripts;
    std::unordered_map<std::string, Texture*> m_textures;
    virtual StaticActorConstructionData* _loadActor(std::string id);
    virtual ISound* _loadAudio(std::string id);
    virtual ISound* _loadAudioStream(std::string id);
    virtual IFont* _loadFont(std::string id);
    virtual Level* _loadLevel(std::string id);
    virtual IModel* _loadModel(std::string id);
    virtual PhysicsMaterial _loadPhysicsMaterial(std::string id);
    virtual GLuint _loadProgram(std::string id);
    virtual IShader* _loadShader(std::string id);
    virtual Material* _loadShaderMaterial(std::string id);
    virtual char* _loadScript(std::string id);
    virtual Texture* _loadTexture(std::string id);

    FT_Library m_font_library;
};

#endif
