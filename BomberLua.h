#ifndef BOMBERLUA_H
#define BOMBERLUA_H

#include <list>
#include <vector>
#include <string>

/**
 * A game engine.
 *
 * The engine is the component that feeds data to the application. This can be
 * done by getting this data from a server, or from computing the state of the
 * game * ourselves via the GameEngine.
 * Elsewhere, the Output module handles the output of the program; it can
 * display the data with SDL and/or send it to clients via the network.
 */
class Engine {

protected:
    int m_iWidth, m_iHeight;

public:
    /**
     * Possible types for the "cells" of the map.
     */
    enum ECell {
        CELL_ROCK,  /**< Indestructible wall */
        CELL_BRICK, /**< Destructible wall */
        CELL_EMPTY  /**< Nothing */
    };

    /**
     * Possible actions for the players.
     */
    enum EAction {
        ACT_IDLE,
        ACT_MOV_LEFT,
        ACT_MOV_RIGHT,
        ACT_MOV_UP,
        ACT_MOV_DOWN,
        ACT_DROP_BOMB
    };

    /**
     * A player.
     */
    class Bomber {

    public:
        /** Name */
        std::string m_sFilename;

        /** Position */
        int m_iPosX, m_iPosY;

        /** Indicates whether the player is alive.
         * A player can be taken out either by getting killed, or by
         * encountering an error (Lua error, or "timeout" while deciding what to
         * do next). */
        bool m_bAlive;

        /** Current action */
        EAction m_eAction;

        /** Date when the action was started */
        double m_dBeginAction;

    public:
        /**
         * Constructor.
         */
        Bomber(const char *filename, int posX, int posY);
    };

    /**
     * A bomb.
     */
    class Bomb {

    public:
        /** Position */
        int m_iPosX, m_iPosY;

        /** Date when the bomb will explode */
        double m_dExplodeDate;

        /** Power (= number of cells that will be affected in each direction) */
        int m_iCells;

    public:
        /**
         * Constructor.
         */
        Bomb(int posX, int posY, double explodeDate, int cells);

    };

public:
    /**
     * Update.
     *
     * Update the data.
     */
    virtual bool update() = 0;

    /** Accessor for map width. */
    inline int getMapWidth()  const { return m_iWidth;  }
    /** Accessor for map height. */
    inline int getMapHeight() const { return m_iHeight; }

    /**
     * Returns the map.
     */
    virtual const std::vector<ECell>& getMap() const = 0;

    /**
     * Returns the players.
     */
    virtual std::vector<const Bomber*> getBombers() const = 0;

    /**
     * Returns the bombs.
     */
    virtual std::vector<const Bomb*> getBombs() const = 0;

    /**
     * Returns the map of the explosions.
     */
    virtual const std::vector<double>& getExplosions() const = 0;

    virtual ~Engine();

    /**
     * Start output.
     *
     * This method can be useful for the engine to "prepare" the objects to be
     * rendered.
     */
    virtual void beginOutput();

    /**
     * End of output.
     *
     * Output is over and the engine can do stuff.
     */
    virtual void endOutput();

};

/**
 * The application.
 *
 * Reads parameters from the command line to initialize the Output and Engine
 * modules.
 */
class BomberLua {

private:
    /**
     * Indicates whether the game is running. If it isn't, the application will
     * terminate.
     */
    bool m_bRunning;

    /**
     * The game engine.
     *
     * Contains the data and offers methods to update them.
     */
    Engine *m_pEngine;

private:
    /**
     * Constructor.
     */
    BomberLua();

    /**
     * Destructor.
     */
    ~BomberLua();

    /**
     * Return a pointer to the unique instance (pattern Singleton).
     */
    static BomberLua *get()
    {
        static BomberLua app;
        return &app;
    }

    /**
     * Initialize the game by reading the command line.
     */
    int init(int argc, char **argv);

    /**
     * Main loop of the application.
     */
    int run();

public:
    /**
     * Start the game (method called from main()).
     */
    static int launch(int argc, char **argv);

};

#endif
