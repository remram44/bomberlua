#ifdef WITH_LUA

#include "LuaBomber.h"

#include <iostream>
#include <sstream>

int LuaBomber::l_log(lua_State *luaState)
{
    int i;
    std::cerr << "Log:";
    for(i = 1; i <= lua_gettop(luaState); i++)
    {
        const char *s = lua_tostring(luaState, i);
        if(s != NULL)
            std::cerr << " " << s;
    }
    std::cerr << "\n";
    return 0;
}

int LuaBomber::l_ready(lua_State *luaState)
{
    // Get a pointer to the associated Bomber
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

    bomber->lock();

    // Mark that the script has acted
    SDL_CondSignal(bomber->m_CondScriptActed);

    // Wait for a signal to resume execution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    return 0;
}

int LuaBomber::l_move(lua_State *luaState)
{
    if(lua_gettop(luaState) != 1)
        luaL_error(luaState, "move(): exactly one parameter expected (not %d)",
            lua_gettop(luaState));
    if(lua_isstring(luaState, 1))
    {
        // Get a pointer to the associated Bomber
        lua_pushstring(luaState, "Bomber");
        lua_gettable(luaState, LUA_REGISTRYINDEX);
        LuaBomber *bomber =
            *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

        bomber->lock();

        std::string param = lua_tostring(luaState, 1);

        bool ret = bomber->move(param);
        lua_pushboolean(luaState, ret);

        // Mark that the script has acted
        SDL_CondSignal(bomber->m_CondScriptActed);

        // Wait for a signal to resume execution
        SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

        bomber->unlock();

        return 1;
    }
    else
    {
        luaL_error(luaState, "move(): the parameter must be a string (and not "
            "%s)", lua_typename(luaState, lua_type(luaState, 1)));
        return 0;
    }
}

int LuaBomber::l_bomb(lua_State *luaState)
{
    // Get a pointer to the associated Bomber
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

    bomber->lock();

    bomber->bomb();

    // Mark that the script has acted
    SDL_CondSignal(bomber->m_CondScriptActed);

    // Wait for a signal to resume execution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    return 0;
}

int LuaBomber::l_get_self(lua_State *luaState)
{
    if(lua_gettop(luaState) != 0)
    {
        luaL_error(luaState, "get_self(): unexpected parameter");
        return 0;
    }

    // Get a pointer to the associated Bomber
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));
    lua_pop(luaState, 1);

    bomber->lock();

    // On crée la table à retourner
    lua_newtable(luaState);
    lua_pushstring(luaState, "posx");
    lua_pushnumber(luaState, bomber->m_iPosX);
    lua_settable(luaState, -3);
    lua_pushstring(luaState, "posy");
    lua_pushnumber(luaState, bomber->m_iPosY);
    lua_settable(luaState, -3);

    bomber->unlock();

    return 1;
}

int LuaBomber::l_get_bombers(lua_State *luaState)
{
    if(lua_gettop(luaState) != 0)
    {
        luaL_error(luaState, "get_bombers(): unexpected parameter");
        return 0;
    }

    // Get a pointer to the associated Bomber
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));
    lua_pop(luaState, 1);

    // Create the table to be returned
    lua_newtable(luaState);

    std::vector<const Engine::Bomber*> bombers =
        bomber->m_pEngine->getBombers();

    int i;
    int n = bombers.size();

    for(i = 0; i < n; i++)
    {
        const Engine::Bomber *it = bombers[i];
        if( (it != bomber) && (it->m_bAlive) )
        {
            lua_pushnumber(luaState, i);

            // Create a table for the bomber
            lua_newtable(luaState);
            lua_pushstring(luaState, "posx");
            lua_pushnumber(luaState, it->m_iPosX);
            lua_settable(luaState, -3);
            lua_pushstring(luaState, "posy");
            lua_pushnumber(luaState, it->m_iPosY);
            lua_settable(luaState, -3);

            // Put it in the returned table
            lua_settable(luaState, -3);
        }
    }

    return 1;
}

int LuaBomber::l_get_field(lua_State *luaState)
{
    if(lua_gettop(luaState) != 0)
    {
        luaL_error(luaState, "get_field(): unexpected parameter");
        return 0;
    }

    // Get a pointer to the associated Bomber
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));
    lua_pop(luaState, 1);

    GameEngine *engine = bomber->m_pEngine;

    // Create the table to be returned
    lua_newtable(luaState);

    // Write the size
    int width = engine->getMapWidth();
    int height = engine->getMapHeight();

    lua_pushstring(luaState, "w");
    lua_pushnumber(luaState, width);
    lua_settable(luaState, -3);
    lua_pushstring(luaState, "h");
    lua_pushnumber(luaState, height);
    lua_settable(luaState, -3);

    // Then loop
    const std::vector<Engine::ECell> &map = engine->getMap();
    std::vector<const Engine::Bomb*> bombs = engine->getBombs();
    const std::vector<double> &explosions = engine->getExplosions();
    int x, y;
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < height; x++)
        {
            std::ostringstream oss;
            oss << x << "," << y;
            lua_pushstring(luaState, oss.str().c_str());
            switch(map[y * width + x])
            {
            // Indestructible wall
            case Engine::CELL_ROCK:
                lua_pushstring(luaState, "rock");
                break;
            // Destructible wall
            case Engine::CELL_BRICK:
                lua_pushstring(luaState, "brick");
                break;
            // No wall; perhaps a bomb or an explosion?
            case Engine::CELL_EMPTY:
                // An explosion
                if(explosions[y * width + x]
                 >= (SDL_GetTicks()/1000.0) )
                    lua_pushstring(luaState, "explosion");
                else
                {
                    // A bomb
                    std::vector<const Engine::Bomb*>::const_iterator bomb;
                    bomb = bombs.begin();
                    for(; bomb != bombs.end(); bomb++)
                    {
                        if( ((*bomb)->m_iPosX == x) && ((*bomb)->m_iPosY == y) )
                        {
                            lua_pushnumber(luaState,
                                (*bomb)->m_dExplodeDate
                                 - (SDL_GetTicks()/1000.0));
                            break;
                        }
                    }
                    if(bomb == bombs.end())
                        lua_pushstring(luaState, "empty");
                }
                break;
            }
            lua_settable(luaState, -3);
        }
    }

    return 1;
}

LuaBomber::LuaBomber(GameEngine *engine, int startx, int starty,
    const char *filename)
  : GameEngine::IABomber(engine, startx, starty, filename),
    m_Thread(NULL), m_LuaState(NULL),
    m_CondScriptActed(NULL), m_CondScriptResume(NULL), m_Mutex(NULL)
{
    // Create the condition used to signal the script is done thinking
    m_CondScriptActed = SDL_CreateCond();

    // Create the condition to signal the engine is waiting for a new action,
    // and that the script should resume execution
    m_CondScriptResume = SDL_CreateCond();

    // Create the mutex
    m_Mutex = SDL_CreateMutex();

    // Create the lua_State
    m_LuaState = lua_open();
    luaL_openlibs(m_LuaState);

    // Remove dangerous functions and tables, that scripts shouldn't need
    const char *const unset[14] = {
        "print", // the log() function should be used, and we might redirect it
        // somewhere else than the application's standard output (for example to
        // separate files)
        "os", // dangerous functions that the script won't need
        "io", // filesystem access
        "debug", // allows access to info about the application
        "package",
        "module",
        "getfenv", // allows access to the caller's environment
        "setfenv",
        "collectgarbage", // lua collects garbage automatically
        "gcinfo",
        "newproxy",
        "dofile", "require", "loadfile" // allows to extend Lua by loading
        // modules
        };

    int i;
    for(i = 0; i < 14; i++)
    {
        lua_pushnil(m_LuaState);
        lua_setglobal(m_LuaState, unset[i]);
    }

    // Place this in the Lua registry
    lua_pushstring(m_LuaState, "Bomber");
    LuaBomber **userdata = static_cast<LuaBomber**>(
        lua_newuserdata(m_LuaState, sizeof(LuaBomber*))
        );
    *userdata = this;
    lua_settable(m_LuaState, LUA_REGISTRYINDEX);

    // Add the functions linking Lua and the game, available during
    // initialization (action functions won't be available before the game
    // starts)
    lua_pushcfunction(m_LuaState, LuaBomber::l_log);
    lua_setglobal(m_LuaState, "log");
    // Add the ready() function
    lua_pushcfunction(m_LuaState, LuaBomber::l_ready);
    lua_setglobal(m_LuaState, "ready");
    // Add the get_field() function, to look at the map
    lua_pushcfunction(m_LuaState, LuaBomber::l_get_field);
    lua_setglobal(m_LuaState, "get_field");

    // Compile the program
    if(luaL_loadfile(m_LuaState, filename) != 0)
    {
        std::cerr << "error loading " << filename << ":\n";
        std::cerr << lua_tostring(m_LuaState, -1) << "\n";
        lua_close(m_LuaState);
        m_LuaState = NULL;
        m_bAlive = false;
        return ;
    }

    lock();

    // Initialization of the script: it has 5s to think and needs to all ready()
    // when done
    m_Thread = SDL_CreateThread(LuaBomber::run, this);

    // The SDL_CondSignal cannot happen before SDL_CondWaitTimeout because the
    // mutex is locked

    int cond_ret = SDL_CondWaitTimeout(m_CondScriptActed, m_Mutex, 5000);
    if(cond_ret != 0)
    {
        std::cerr << "Initialization failed (script killed after 5 seconds)\n";

        // If the script takes too much time, we kill it
        SDL_KillThread(m_Thread);
        m_Thread = NULL;
        //lua_close(m_LuaState); we killed the script, the context Lua shouldn't
        // be used anymore (this leaks...)
        m_LuaState = NULL;
        m_bAlive = false;
    }
    else
    {
        // Remove the ready() function
        lua_pushnil(m_LuaState);
        lua_setglobal(m_LuaState, "ready");

        // Add the get_self() function to get info about oneself
        lua_pushcfunction(m_LuaState, LuaBomber::l_get_self);
        lua_setglobal(m_LuaState, "get_self");
        // Add the get_bombers() function to look at the other players
        lua_pushcfunction(m_LuaState, LuaBomber::l_get_bombers);
        lua_setglobal(m_LuaState, "get_bombers");
        // Add the move() function to move around
        lua_pushcfunction(m_LuaState, LuaBomber::l_move);
        lua_setglobal(m_LuaState, "move");
        // Add the bomb() function to plant a bomb
        lua_pushcfunction(m_LuaState, LuaBomber::l_bomb);
        lua_setglobal(m_LuaState, "bomb");
    }

    unlock();
}

LuaBomber::~LuaBomber()
{
    if(m_Thread)
        SDL_KillThread(m_Thread);
    if(m_LuaState)
        lua_close(m_LuaState);
    if(m_CondScriptActed)
        SDL_DestroyCond(m_CondScriptActed);
    if(m_CondScriptResume)
        SDL_DestroyCond(m_CondScriptResume);
    if(m_Mutex)
        SDL_DestroyMutex(m_Mutex);
}

void LuaBomber::update()
{
    // Send a signal to make the thread resume
    SDL_CondSignal(m_CondScriptResume);
}

int LuaBomber::run(void *data)
{
    LuaBomber *bomber = static_cast<LuaBomber*>(data);

    // Start the Lua script
    lua_State *luaState = bomber->m_LuaState;
    if(lua_pcall(luaState, 0, 0, 0) != 0)
        std::cerr << "\"" << bomber->m_sFilename << "\" exited with an error:\n"
            "    " << lua_tostring(luaState, -1) << "\n";
    else
        std::cerr << "\"" << bomber->m_sFilename << "\" exited without error\n";

    bomber->lock();

    bomber->m_bAlive = false;

    bomber->unlock();

    return 0;
}

void LuaBomber::kill()
{
    if(m_Thread)
    {
        SDL_KillThread(m_Thread);
        m_Thread = NULL;
    }
    m_bAlive = false;
}

void LuaBomber::lock()
{
    SDL_LockMutex(m_Mutex);
}

void LuaBomber::unlock()
{
    SDL_UnlockMutex(m_Mutex);
}

#endif
