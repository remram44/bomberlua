#include "NetworkReceiver.h"

NetworkReceiver::NetworkReceiver(const char *address, int port)
{
    // Connexion au serveur
    m_Socket.Connect(address, port);
}

bool NetworkReceiver::update()
{
    // TODO : Mise à jour des données
    return false;
}

const std::vector<Engine::ECell>& NetworkReceiver::getMap() const
{
    return m_Map;
}

std::vector<const Engine::Bomber*> NetworkReceiver::getBombers() const
{
    std::vector<const Engine::Bomber*> bombers;
    bombers.assign(m_Bombers.begin(), m_Bombers.end());
    return bombers;
}

std::vector<const Engine::Bomb*> NetworkReceiver::getBombs() const
{
    std::vector<const Engine::Bomb*> bombs;
    bombs.assign(m_Bombs.begin(), m_Bombs.end());
    return bombs;
}

const std::vector<double>& NetworkReceiver::getExplosions() const
{
    return m_Explosions;
}
