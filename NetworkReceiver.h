#ifndef NETWORKRECEIVER_H
#define NETWORKRECEIVER_H

#include "BomberLua.h"
#include "Socket.h"
#include <vector>
#include <list>

/**
 * Network engine.
 *
 * This engine doesn't compute the game state but simply receives it from a
 * server.
 */
class NetworkReceiver : public Engine {

private:
    /** Socket connected to the server */
    ClientSocket m_Socket;

    /** The map */
    std::vector<Engine::ECell> m_Map;

    /** The players */
    std::vector<Engine::Bomber*> m_Bombers;

    /** The bombs */
    std::list<Engine::Bomb*> m_Bombs;

    /** The explosions */
    std::vector<double> m_Explosions;

public:
    /**
     * Constructor.
     *
     * Connect to a server that will provide the game data.
     */
    NetworkReceiver(const char *address, int port);

    /**
     * Update the data.
     */
    bool update();

    /**
     * Return the map.
     */
    const std::vector<Engine::ECell>& getMap() const;

    /**
     * Return the players.
     */
    std::vector<const Engine::Bomber*> getBombers() const;

    /**
     * Return the bombs.
     */
    std::vector<const Engine::Bomb*> getBombs() const;

    /**
     * Return the explosions.
     */
    const std::vector<double>& getExplosions() const;

};

#endif
