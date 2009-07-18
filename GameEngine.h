#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "BomberLua.h"
#include <list>
#include <vector>
#include <string>

#ifdef WITH_LUA
class LuaBomber;
#endif
#ifdef WITH_PYTHON
class PyBomber;
#endif

/**
 * \defgroup gameengine The game's engine
 */
/** @{ */
/**
 * The game engine.
 *
 * This engine is the "main" engine, which computes the data by making the AIs
 * fight.
 */
class GameEngine : public Engine {

public:
    /**
     * A script-controller player.
     */
    class IABomber : public Engine::Bomber {

    protected:
        GameEngine *m_pEngine;

    protected:
        void wait();
        bool move(const std::string &param);
        void bomb();

    public:
        /**
         * Constructor.
         *
         * Load the script and initialize it.
         * @param filename Name of the file to use.
         */
        IABomber(GameEngine *engine, int startx, int starty,
            const char *filename);

        /**
         * Destructor.
         */
        virtual ~IABomber() {}

        /**
         * Resume execution of the script in a thread.
         */
        virtual void update() = 0;

        /**
         * Kill this player.
         *
         * He is then no longer alive, and the thread is killed if it was
         * running.
         */
        virtual void kill() = 0;

        /**
         * Lock this player.
         *
         * Accessing attributes of the player requires it to be locked, since
         * they can be accessed from threads.
         */
        virtual void lock() = 0;

        /**
         * Unlock the player.
         */
        virtual void unlock() = 0;

    };

private:
    std::vector<Engine::ECell> m_Map;
    std::vector<IABomber*> m_Bombers;
    std::list<Engine::Bomb*> m_Bombs;
    std::vector<double> m_Explosions;

public:
    /**
     * Accessor for the map.
     * 
     * Allows one to get data on the cells of the map.
     */
    const std::vector<Engine::ECell>& getMap() const;

    /**
     * Accessor for the players.
     */
    std::vector<const Engine::Bomber*> getBombers() const;

    /**
     * Accessor for the bombs.
     */
    std::vector<const Engine::Bomb*> getBombs() const;

    /**
     * Accessor for the explosions.
     */
    const std::vector<double>& getExplosions() const;

public:
    /**
     * Constructor.
     *
     * @param progs List of programs to load.
     */
    GameEngine(std::list<std::string> progs);

    /**
     * Destructor.
     */
    ~GameEngine();

    /**
     * Update by executing one step.
     *
     * @return false if the match is over, true otherwise.
     */
    bool update();

    /**
     * Begining of output.
     *
     * Method overloaded to lock all the players.
     */
    void beginOutput();

    /**
     * End of output.
     *
     * Method overloaded to unlock all the players.
     */
    void endOutput();

};
/** @} */

#ifdef WITH_LUA
#include "LuaBomber.h"
#endif
#ifdef WITH_PYTHON
#include "PyBomber.h"
#endif

#endif
