#ifndef LUABOMBER_H
#define LUABOMBER_H

#include "GameEngine.h"

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "lua.hpp"

/**
 * A player controller from a Lua script.
 */
class LuaBomber : public GameEngine::IABomber {

private:
    /** Function to log information */
    static int l_log(lua_State *);
    
    /** Function allowing a script to indicate it's ready to start the game */
    static int l_ready(lua_State *);

    /** Function allowing a script to move; the parameter must be "left", "right", "up" or "down" */
    static int l_move(lua_State *);

    /** Function to plant a bomb */
    static int l_bomb(lua_State *);

    /**
     * Function returning information about oneself as a table.
     *
     * The structure of the table representing a player is as follow:
     *  - posx, posy: position of the player.
     */
    static int l_get_self(lua_State *);

    /**
     * Function returning the players as a table.
     *
     * Each player is identified with a unique number, and is represented by a
     * table (see l_get_self()).
     */
    static int l_get_bombers(lua_State *);

    /**
     * Function returning information about the map as a table.
     *
     * The attributes w and h indicate its size, and to each string "x,y" is
     * associated a type of cell, which can be:
     *  - "rock": indestructible wall
     *  - "brick": wall that can be destroyed by a bomb
     *  - a number: not a wall, but a bomb that will explode in that much time
     * (in seconds)
     *  - "explosion": not a wall, but an explosion
     *  - "empty": nothing
     */
    static int l_get_field(lua_State *);

private:
    /** AI's thread. */
    SDL_Thread *m_Thread;

    /** Lua context of the script. */
    lua_State *m_LuaState;

    /** Condition used to indicate the end of a script's execution. */
    SDL_cond *m_CondScriptActed;

    /** Condition used to make the script resume execution. */
    SDL_cond *m_CondScriptResume;

    /** Mutex */
    SDL_mutex *m_Mutex;

private:
    /**
     * Function started in a thread to execute the Lua script..
     *
     * @param data lua_State to execute.
     */
    static int run(void *data);

public:
    /**
     * Constructor.
     *
     * Compiles the Lua script.
     * @param filename Name of the file to use.
     */
    LuaBomber(GameEngine *engine, int startx, int starty,
        const char *filename);

    /**
     * Destructor.
     */
    ~LuaBomber();

    /**
     * Resume execution of the script in a thread.
     */
    void update();

    /**
     * Kill this player.
     *
     * He is then no longer alive, and the thread is killed if it was running.
     */
    void kill();

    /**
     * Lock this player.
     *
     * Accessing attributes of the player requires it to be locked, since they
     * can be accessed from threads.
     */
    void lock();

    /**
     * Unlock the player.
     */
    void unlock();

};

#endif
