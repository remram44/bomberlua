#ifndef LUABOMBER_H
#define LUABOMBER_H

#include "GameEngine.h"

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "lua.hpp"

/**
 * Un joueur controllé par un script Lua.
 */
class LuaBomber : public GameEngine::IABomber {

private:
    /** Fonction permettant à un script Lua d'enregistrer des informations */
    static int l_log(lua_State *);
    
    /** Fonction permettant à un script d'indiquer qu'il est initialisé et prêt
     * à commencer la partie */
    static int l_ready(lua_State *);

    /** Fonction permettant à un script d'indiquer qu'il ne fait rien mais qu'il
     * n'est pas planté */
    static int l_wait(lua_State *);

    /** Fonction permettant de se déplacer ; le paramètre doit être "left",
     * "right", "up" ou "down" */
    static int l_move(lua_State *);

    /** Fonction permettant de poser une bombe. */
    static int l_bomb(lua_State *);

    /**
     * Fonction retournant les informations sur soi-même sous forme d'une table.
     *
     * La structure d'une table décrivant un personnage est la suivante :
     *  - posx, posy : la position du personnage.
     */
    static int l_get_self(lua_State *);

    /**
     * Fonction retournant les personnages, sous forme d'une table.
     *
     * Chaque personnage est associé à un nombre entier unique, et est lui-
     * même représenté par une table (voir l_get_self()).
     */
    static int l_get_bombers(lua_State *);

    /**
     * Fonction retournant les informations sur la map sous forme d'une
     * table.
     *
     * Les champs w et h indiquent sa taille, et à chaque chaîne "x,y" est
     * associée un type de cellule, qui peut être :
     *  - "rock" : un mur indestructible
     *  - "brick" : un mur qui peut être détruit par une bombe
     *  - un nombre : pas de mur, mais une bombe qui explosera dans cette durée
     * (en secondes)
     *  - "explosion" : pas de mur, mais une explosion
     *  - "empty" : rien
     */
    static int l_get_field(lua_State *);

private:
    /** Thread de l'IA. */
    SDL_Thread *m_Thread;

    /** Contexte Lua du script. */
    lua_State *m_LuaState;

    /** Condition utilisée pour indiquer la fin de l'exécution du script. */
    SDL_cond *m_CondScriptActed;

    /** Condition utilisée pour provoquer la reprise de l'exécution du
     * script. */
    SDL_cond *m_CondScriptResume;

    /** Mutex */
    SDL_mutex *m_Mutex;

private:
    /**
     * Fonction lancée dans un thread pour exécuter un script Lua.
     *
     * @param data lua_State à exécuter.
     */
    static int run(void *data);

public:
    /**
     * Constructeur.
     *
     * Compile le script Lua.
     * @param filename Nom du fichier Lua à utiliser.
     */
    LuaBomber(GameEngine *engine, int startx, int starty,
        const char *filename);

    /**
     * Destructeur.
     */
    ~LuaBomber();

    /**
     * Reprend l'exécution du script Lua dans un thread.
     */
    void update();

    /**
     * Tue ce personnage.
     *
     * Il n'est alors plus vivant, et le thread est tué s'il était actif.
     */
    void kill();

    /**
     * Verrouille le personnage.
     *
     * Tout accès aux membres du personnage nécessite son verrouillage, car ils
     * sont succeptibles d'être utilisés également depuis les threads.
     */
    void lock();

    /**
     * Déverrouille le personnage.
     */
    void unlock();

};

#endif
