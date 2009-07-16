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
    // On récupère un pointeur sur le Bomber associé
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

    bomber->lock();

    // On indique que le script a agit
    SDL_CondSignal(bomber->m_CondScriptActed);

    // On attend un signal pour reprendre l'exécution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    return 0;
}

int LuaBomber::l_move(lua_State *luaState)
{
    if(lua_gettop(luaState) != 1)
        luaL_error(luaState, "move(): exactement un paramètre attendu ("
            "et non %d)", lua_gettop(luaState));
    if(lua_isstring(luaState, 1))
    {
        // On récupère un pointeur sur le Bomber associé
        lua_pushstring(luaState, "Bomber");
        lua_gettable(luaState, LUA_REGISTRYINDEX);
        LuaBomber *bomber =
            *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

        bomber->lock();

        std::string param = lua_tostring(luaState, 1);

        if(!bomber->move(param))
            luaL_error(luaState, "move(): paramètre \"%s\" non reconnu",
                param.c_str());

        // On indique que le script a agit
        SDL_CondSignal(bomber->m_CondScriptActed);

        // On attend un signal pour reprendre l'exécution
        SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

        bomber->unlock();
    }
    else
        luaL_error(luaState, "move(): le paramètre doit être de type string ("
            "et non %s)", lua_typename(luaState, lua_type(luaState, 1)));

    return 0;
}

int LuaBomber::l_bomb(lua_State *luaState)
{
    // On récupère un pointeur sur le Bomber associé
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));

    bomber->lock();

    bomber->bomb();

    // On indique que le script a agit
    SDL_CondSignal(bomber->m_CondScriptActed);

    // On attend un signal pour reprendre l'exécution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    return 0;
}

int LuaBomber::l_get_self(lua_State *luaState)
{
    if(lua_gettop(luaState) != 0)
    {
        luaL_error(luaState, "get_self(): paramètre inattendu");
        return 0;
    }

    // On récupère un pointeur sur le Bomber associé
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
        luaL_error(luaState, "get_bombers(): paramètre inattendu");
        return 0;
    }

    // On récupère un pointeur sur le Bomber associé
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));
    lua_pop(luaState, 1);

    // On crée la table à retourner
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

            // On crée la table du bomber
            lua_newtable(luaState);
            lua_pushstring(luaState, "posx");
            lua_pushnumber(luaState, it->m_iPosX);
            lua_settable(luaState, -3);
            lua_pushstring(luaState, "posy");
            lua_pushnumber(luaState, it->m_iPosY);
            lua_settable(luaState, -3);

            // On la place dans la table à retourner
            lua_settable(luaState, -3);
        }
    }
    
    return 1;
}

int LuaBomber::l_get_field(lua_State *luaState)
{
    if(lua_gettop(luaState) != 0)
    {
        luaL_error(luaState, "get_field(): paramètre inattendu");
        return 0;
    }

    // On récupère un pointeur sur le Bomber associé
    lua_pushstring(luaState, "Bomber");
    lua_gettable(luaState, LUA_REGISTRYINDEX);
    LuaBomber *bomber =
        *static_cast<LuaBomber**>(lua_touserdata(luaState, -1));
    lua_pop(luaState, 1);

    GameEngine *engine = bomber->m_pEngine;

    // On crée la table à retourner
    lua_newtable(luaState);

    // On y place la taille
    int width = engine->getMapWidth();
    int height = engine->getMapHeight();

    lua_pushstring(luaState, "w");
    lua_pushnumber(luaState, width);
    lua_settable(luaState, -3);
    lua_pushstring(luaState, "h");
    lua_pushnumber(luaState, height);
    lua_settable(luaState, -3);

    // Puis on boucle
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
            // Mur indestructible
            case Engine::CELL_ROCK:
                lua_pushstring(luaState, "rock");
                break;
            // Mur destructible
            case Engine::CELL_BRICK:
                lua_pushstring(luaState, "brick");
                break;
            // Pas de mur ; mais peut-être une bombe ou une explosion ?
            case Engine::CELL_EMPTY:
                // Une explosion
                if(explosions[y * width + x]
                 >= (SDL_GetTicks()/1000.0) )
                    lua_pushstring(luaState, "explosion");
                else
                {
                    // Une bombe
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
    // Crée la condition indiquant que le script a fini son exécution
    m_CondScriptActed = SDL_CreateCond();

    // Crée la condition indiquant que le moteur attend une nouvelle action, et
    // que le script doit reprendre son exécution
    m_CondScriptResume = SDL_CreateCond();

    // Crée le mutex
    m_Mutex = SDL_CreateMutex();

    // Crée le lua_State
    m_LuaState = lua_open();
    luaL_openlibs(m_LuaState);

    // On vire les fonctions et tables dangereuses, dont les scripts n'ont pas
    // besoin
    const char *const unset[14] = {
        "print", // la fonction log() doit être utilisée, elle pourrait être
        // redirigée ailleurs que sur la sortie du programme (par exemple dans
        // des fichiers séparés)
        "os", // fonctions dangereuses et inutiles au script
        "io", // permet d'accéder au système de fichiers
        "debug", // permet d'accéder à des informations sur le programme
        "package",
        "module",
        "getfenv", // permet d'accéder à l'environnement de l'appelant
        "setfenv",
        "collectgarbage", // lua collectera les détritus automatiquement
        "gcinfo",
        "newproxy",
        "dofile", "require", "loadfile" // permet d'étendre Lua en chargeant
        // des modules
        };

    int i;
    for(i = 0; i < 14; i++)
    {
        lua_pushnil(m_LuaState);
        lua_setglobal(m_LuaState, unset[i]);
    }

    // Place this dans le registre de Lua
    lua_pushstring(m_LuaState, "Bomber");
    LuaBomber **userdata = static_cast<LuaBomber**>(
        lua_newuserdata(m_LuaState, sizeof(LuaBomber*))
        );
    *userdata = this;
    lua_settable(m_LuaState, LUA_REGISTRYINDEX);

    // Ajout des fonctions faisant le lien entre Lua et le programme,
    // disponibles lors de l'initialisation (les fonctions d'action ne sont pas
    // disponibles avant le lancement du jeu)
    lua_pushcfunction(m_LuaState, LuaBomber::l_log);
    lua_setglobal(m_LuaState, "log");
    // Ajout de la fonction ready()
    lua_pushcfunction(m_LuaState, LuaBomber::l_ready);
    lua_setglobal(m_LuaState, "ready");
    // Ajout de la fonction get_field(), permettant de regarder la map
    lua_pushcfunction(m_LuaState, LuaBomber::l_get_field);
    lua_setglobal(m_LuaState, "get_field");

    // Compile le programme
    if(luaL_loadfile(m_LuaState, filename) != 0)
    {
        std::cerr << "erreur au chargement de " << filename << " :\n";
        std::cerr << lua_tostring(m_LuaState, -1) << "\n";
        lua_close(m_LuaState);
        m_LuaState = NULL;
        m_bAlive = false;
        return ;
    }

    lock();

    // Initialisation du script : il a 5s pour s'initialiser et doit appeler
    // ready() lorsqu'il est prêt
    m_Thread = SDL_CreateThread(LuaBomber::run, this);

    // Le SDL_CondSignal ne peut pas avoir lieu avant l'appel à
    // SDL_CondWaitTimeout car le Mutex est locké

    int cond_ret = SDL_CondWaitTimeout(m_CondScriptActed, m_Mutex, 5000);
    if(cond_ret != 0)
    {
        std::cerr << "Initialisation ratée (script killé après 5 secondes)\n";

        // Si le script prend trop de temps, on le kill
        SDL_KillThread(m_Thread);
        m_Thread = NULL;
        //lua_close(m_LuaState); on a killé l'exécution du script, le contexte
        // Lua ne doit plus être utilisé (tant pis pour le leak...)
        m_LuaState = NULL;
        m_bAlive = false;
    }
    else
    {
        // Enlève la fonction ready()
        lua_pushnil(m_LuaState);
        lua_setglobal(m_LuaState, "ready");

        // Ajout de la fonction get_self() permettant d'avoir des infos sur
        // soi-même
        lua_pushcfunction(m_LuaState, LuaBomber::l_get_self);
        lua_setglobal(m_LuaState, "get_self");
        // Ajout de la fonction get_bombers() permettant de voir les autres
        // personnages
        lua_pushcfunction(m_LuaState, LuaBomber::l_get_bombers);
        lua_setglobal(m_LuaState, "get_bombers");
        // Ajout de move() pour se déplacer
        lua_pushcfunction(m_LuaState, LuaBomber::l_move);
        lua_setglobal(m_LuaState, "move");
        // Ajout de bomb() pour poser une bombe
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
    // On envoie un signal pour indiquer au thread de reprendre
    SDL_CondSignal(m_CondScriptResume);
}

int LuaBomber::run(void *data)
{
    LuaBomber *bomber = static_cast<LuaBomber*>(data);

    // Lance le script Lua
    lua_State *luaState = bomber->m_LuaState;
    if(lua_pcall(luaState, 0, 0, 0) != 0)
        std::cerr << "\"" << bomber->m_sFilename << "\" s'est terminé avec une "
            "erreur :\n"
            "    " << lua_tostring(luaState, -1) << "\n";
    else
        std::cerr << "\"" << bomber->m_sFilename << "\" s'est terminé sans "
            "erreur\n";

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
