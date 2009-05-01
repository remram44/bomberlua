#ifndef NETWORKRECEIVER_H
#define NETWORKRECEIVER_H

#include "BomberLua.h"
#include "Socket.h"
#include <vector>
#include <list>

/**
 * Moteur réseau.
 *
 * Ce moteur ne calcule pas l'état du jeu mais le reçoit simplement depuis un
 * serveur.
 */
class NetworkReceiver : public Engine {

private:
    /** Socket connectée au serveur */
    ClientSocket m_Socket;

    /** La map */
    std::vector<Engine::ECell> m_Map;

    /** Les personnages */
    std::vector<Engine::Bomber*> m_Bombers;

    /** Les bombes */
    std::list<Engine::Bomb*> m_Bombs;

    /** Les explosions */
    std::vector<double> m_Explosions;

public:
    /**
     * Constructeur.
     *
     * Se connecte à un serveur qui fournira les informations sur le jeu.
     */
    NetworkReceiver(const char *address, int port);

    /**
     * Met à jour les données.
     */
    bool update();

    /**
     * Retourne la map.
     */
    const std::vector<Engine::ECell>& getMap() const;

    /**
     * Retourne les personnages.
     */
    std::vector<const Engine::Bomber*> getBombers() const;

    /**
     * Retourne les bombes.
     */
    std::vector<const Engine::Bomb*> getBombs() const;

    /**
     * Retourne les explosions.
     */
    const std::vector<double>& getExplosions() const;

};

#endif
