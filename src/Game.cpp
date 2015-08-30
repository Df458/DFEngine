#include "Actor.h"
#include "ActorSystem.h"
#include "AudioSystem.h"
#include "ComponentFactory.h"
#include "CGraphics.h"
#include "CRigidBody.h"
#include "CScript.h"
#include "EventSystem.h"
#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "Level.h"
#include "PhysicsRenderer.h"
#include "PhysicsSystem.h"
#include "ResourceManager.h"
#include "ResourceDefines.h"
#include "RenderUtil.h"
#include "Scene.h"
#include "Sound.h"
#include "TweenSystem.h"
#include "Game.h"
#include "Util.h"

Game::Game(void)
{

}

bool Game::initialize(void)
{
    if(!glfwInit()) {
        error("Failed to intialize GLFW.");
        return false;
    }

    m_events = new EventSystem();
    m_actors = new ActorSystem();
    m_audio = new AudioSystem();
    m_graphics = new GraphicsSystem();
    m_physics = new PhysicsSystem();
    m_tweens = new TweenSystem();
    m_resources = new DFBaseResourceManager();
    m_resources = new DFBaseResourceManager();
    m_components = new ComponentFactory();

    if(!m_events->initialize()) {
        delete m_resources;
        delete m_tweens;
        delete m_actors;
        delete m_physics;
        delete m_graphics;
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the EventSystem.");
        return false;
    }

    if(!m_graphics->initialize()) {
        delete m_audio;
        delete m_resources;
        delete m_tweens;
        delete m_actors;
        delete m_physics;
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the GraphicsSystem.");
        return false;
    }

    if(!m_physics->initialize()) {
        delete m_audio;
        delete m_resources;
        delete m_tweens;
        delete m_actors;
        delete m_physics;
        m_graphics->cleanup();
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the PhysicsSystem.");
        return false;
    }

    if(!m_actors->initialize()) {
        delete m_audio;
        delete m_resources;
        delete m_tweens;
        delete m_actors;
        m_physics->cleanup();
        delete m_physics;
        m_graphics->cleanup();
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the ActorSystem.");
        return false;
    }

    if(!m_tweens->initialize()) {
        delete m_audio;
        delete m_resources;
        m_actors->cleanup();
        delete m_actors;
        m_physics->cleanup();
        delete m_physics;
        m_graphics->cleanup();
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the TweenSystem.");
        return false;
    }

    if(!m_resources->initialize()) {
        delete m_audio;
        delete m_resources;
        m_tweens->cleanup();
        delete m_tweens;
        m_actors->cleanup();
        delete m_actors;
        m_physics->cleanup();
        delete m_physics;
        m_graphics->cleanup();
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the ResourceManager.");
        return false;
    }

    m_graphics->init_letterbox();

    if(!m_audio->initialize()) {
        delete m_audio;
        m_resources->cleanup();
        delete m_resources;
        m_tweens->cleanup();
        delete m_tweens;
        m_actors->cleanup();
        delete m_actors;
        m_physics->cleanup();
        delete m_physics;
        m_graphics->cleanup();
        delete m_graphics;
        m_events->cleanup();
        delete m_events;
        glfwTerminate();
        error("Failed to initialize the AudioSystem.");
        return false;
    }

    m_components->registerComponentBuilder(buildRigidBody, "rigidbody");
    m_components->registerComponentBuilder(buildGraphics, "graphics");
    m_components->registerComponentBuilder(buildCamera, "camera");
    m_components->registerComponentBuilder(buildScript, "script");

    warn("Disregard this warning. All systems normal.");

    return true;
}

void Game::mainLoop(void)
{
    glfwSetTime(0);
    do {
        m_delta_time = glfwGetTime();
        glfwSetTime(0);
        m_physics->update(m_delta_time);
        m_input->update(m_delta_time);
        glfwPollEvents();
        m_events->update(m_delta_time);
        m_tweens->update(m_delta_time);
        m_actors->update(m_delta_time);
        m_graphics->render();
    } while(!m_quit);
}

void Game::cleanup(void)
{
    delete m_components;

    m_resources->cleanup();
    delete m_resources;

    m_audio->cleanup();
    delete m_audio;

    m_actors->cleanup();
    delete m_actors;

    m_tweens->cleanup();
    delete m_tweens;

    m_physics->cleanup();
    delete m_physics;

    m_graphics->cleanup();
    delete m_graphics;

    m_events->cleanup();
    delete m_events;
    glfwTerminate();
}

void Game::quit()
{
    m_quit = true;
}

bool Game::buildLevel(std::string level, bool keep_actors)
{
    if(!keep_actors)
        m_actors->softClear();
    Level* level_data = m_resources->getLevel(level);
    if(!level_data) {
        error("Couldn't load the requested level.");
        return false;
    }
    for(auto i : level_data->getActorList())
        m_actors->createActor(i);
    m_graphics->loadSceneFromLevel(level_data);
    m_physics->setGravity(level_data->getGravity());
    m_physics->setWorldScale(level_data->getWorldScale());
    m_graphics->getActiveScene()->updateViewportSize();
    return true;
}

DFBaseGame::DFBaseGame(void)
{
}

bool DFBaseGame::initialize(void)
{
    if(!Game::initialize())
        return false;
    m_input = new DFBaseInputSystem(m_graphics, m_events);

    if(!m_input->initialize()) {
        error("Failed to initialize the InputSystem.");
        Game::cleanup();
        return false;
    }

    PhysicsRenderer* pr = new PhysicsRenderer();
    m_graphics->setPhysicsDebug(pr);
    m_physics->setPhysicsDebug(pr);

    m_graphics->setTitle("DFBase Main Window");

    if(!buildLevel("main")) {
        Game::cleanup();
        m_input->cleanup();
        delete m_input;
        return false;
    }

    //m_resources->getAudio("Seagull-hit")->play();

    return true;
}

void DFBaseGame::cleanup(void)
{
    m_input->cleanup();
    delete m_input;
    Game::cleanup();
}

int game_create_actor(lua_State* state)
{
    Actor* actor = g_game->actors()->createActor(state);
    if(!actor) {
        // TODO: Error here
        warn("Trying to retrieve an actor that doesn't exist!");
        return 0;
    }

    lua_newtable(state);
    luaL_setfuncs(state, actor_funcs, 0);
    Actor** actordat = static_cast<Actor**>(lua_newuserdata(state, sizeof(Actor*)));
    *actordat = actor;
    lua_setfield(state, -2, "instance");

    return 1;
}

int game_get_actor(lua_State* state)
{
    Actor* actor = NULL;
    if(lua_isinteger(state, 1))
        actor = g_game->actors()->getActor((unsigned)lua_tointeger(state, 1));
    else
        actor = g_game->actors()->getActor(lua_tostring(state, 1));
    if(!actor) {
        return 0;
    }

    lua_newtable(state);
    luaL_setfuncs(state, actor_funcs, 0);
    Actor** actordat = static_cast<Actor**>(lua_newuserdata(state, sizeof(Actor*)));
    *actordat = actor;
    lua_setfield(state, -2, "instance");

    return 1;
}

int game_get_actors(lua_State* state)
{
    std::vector<Actor*> actors;
    actors = g_game->actors()->getActors(lua_tostring(state, 1));
    if(actors.size() == 0) {
        return 0;
    }

    lua_createtable(state, actors.size(), 0);
    for(unsigned long i = 0; i < actors.size(); ++i) {
        lua_pushinteger(state, i);
        lua_newtable(state);
        luaL_setfuncs(state, actor_funcs, 0);
        Actor** actordat = static_cast<Actor**>(lua_newuserdata(state, sizeof(Actor*)));
        *actordat = actors[i];
        lua_setfield(state, -2, "instance");
        lua_settable(state, -3);
    }

    return 1;
}

int game_exit(lua_State* state)
{
    g_game->quit();
    return 0;
}

int game_load_level(lua_State* state)
{
    bool keep_actors = false;
    std::string level_name = lua_tostring(state, 1);
    if(lua_gettop(state) > 1)
        keep_actors = lua_toboolean(state, 2);
    if(!g_game->buildLevel(level_name, keep_actors))
        error("Failed to load level");
    // TODO: Terminate the calling function to prevent issues, and clear actors
    // in post

    return 0;
}

int game_debug_render(lua_State* state)
{
    bool show = lua_toboolean(state, 1);
    if(show)
        g_game->graphics()->getPhysicsDebug()->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb);
    else
        g_game->graphics()->getPhysicsDebug()->setDebugMode(0);
    //g_game->graphics()->getPhysicsDebug()->setDebugMode(btIDebugDraw::DBG_DrawText);
    return 0;
}

int game_get_data_path(lua_State* state)
{
    lua_pushstring(state, (getPath() + "/" + DATA_PATH).c_str());
    return 1;
}
