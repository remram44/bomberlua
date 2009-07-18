#include "GameEngine.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <limits>

GameEngine::GameEngine(std::list<std::string> progs)
{
    // Initialisation du monde (création de la map)
    m_iWidth = 11;
    m_iHeight = 11;
    m_Map.resize(m_iWidth * m_iHeight);
    int x, y;
    for(y = 0; y < 11; y++)
    {
        for(x = 0; x < 11; x++)
        {
            if( (x == 0) || (x == 10) || (y == 0) || (y == 10) )
                m_Map[y * m_iWidth + x] = Engine::CELL_ROCK;
            else if( (x % 2 == 0) && (y % 2 == 0) )
                m_Map[y * m_iWidth + x] = Engine::CELL_ROCK;
            else if( ( (x <= 2) || (x > 7) )
             && ( (y <= 2) || (y > 7) ) )
                m_Map[y * m_iWidth + x] = Engine::CELL_EMPTY;
            else
                m_Map[y * m_iWidth + x] = Engine::CELL_BRICK;
        }
    }
    {
        m_Explosions.resize(m_iWidth * m_iHeight);
        double n_inf = -std::numeric_limits<double>::infinity();
        std::fill(m_Explosions.begin(), m_Explosions.end(), n_inf);
    }

    // Création des personnages + chargement des programmes
    m_Bombers.resize(progs.size());
    int i = 0;
    std::list<std::string>::const_iterator it = progs.begin();
#ifdef WITH_PYTHON
    bool py_loaded = false; // Limitation de Python - un seul bot python
#endif
    for(; it != progs.end(); i++, it++)
    {
        int startx, starty;
        switch(i)
        {
        case 0:
            startx = 1; starty = 1;
            break;
        case 1:
            startx = 9; starty = 1;
            break;
        case 2:
            startx = 1; starty = 9;
            break;
        case 3:
            startx = 9; starty = 9;
            break;
        default:
            // TODO : 4 joueurs max pour l'instant
            std::cerr << "Trop de joueurs !\n";
            exit(1);
            break;
        }
        const std::string &file = *it;
#ifdef WITH_LUA
        if(file.substr(file.size() - 4) == ".lua")
            m_Bombers[i] = new LuaBomber(this, startx, starty, file.c_str());
        else
#endif
#ifdef WITH_PYTHON
        if(file.substr(file.size() - 3) == ".py")
        {
            if(!py_loaded)
            {
                m_Bombers[i] = new PyBomber(this, startx, starty, file.c_str());
                py_loaded = true;
            }
            else
            {
                std::cerr << file << " : impossible de charger plus d'un "
                    "script Python ! (limitation au niveau de la lib Python...)"
                    "\n";
                exit(1);
            }
        }
        else
#endif
        {
            std::cerr << file << " : extension non-reconnue\n";
            exit(1);
        }
    }
}

GameEngine::~GameEngine()
{
    std::vector<IABomber*>::iterator it = m_Bombers.begin();
    for(; it != m_Bombers.end(); it++)
    {
        delete *it;
    }
}

bool GameEngine::update()
{
    int nbombers = 0;

    // Boucle sur les personnages
    std::vector<IABomber*>::iterator perso = m_Bombers.begin();
    for(; perso != m_Bombers.end(); perso++)
    {
        // On le lock
        (*perso)->lock();

        // Si ce personnage n'est pas mort
        if((*perso)->m_bAlive)
        {
            nbombers++;

            // Le perso se fait-il toucher par une explosion ?
            bool bExplode = m_Explosions[(*perso)->m_iPosY * m_iWidth
                + (*perso)->m_iPosX] > getTicks();

            if(!bExplode)
                switch((*perso)->m_eAction)
                {
                case Engine::ACT_MOV_LEFT:
                    bExplode = m_Explosions[(*perso)->m_iPosY * m_iWidth
                        + ( (*perso)->m_iPosX - 1)] > getTicks();
                    break;
                case Engine::ACT_MOV_UP:
                    bExplode = m_Explosions[( (*perso)->m_iPosY - 1) * m_iWidth
                        + (*perso)->m_iPosX] > getTicks();
                    break;
                case Engine::ACT_MOV_RIGHT:
                    bExplode = m_Explosions[(*perso)->m_iPosY * m_iWidth
                        + ( (*perso)->m_iPosX + 1)] > getTicks();
                    break;
                case Engine::ACT_MOV_DOWN:
                    bExplode = m_Explosions[( (*perso)->m_iPosY + 1) * m_iWidth
                        + (*perso)->m_iPosX] > getTicks();
                    break;
                default:
                    break;
                }

            if(bExplode)
                (*perso)->m_bAlive = false;
        }

        // S'il n'est toujours pas mort, il bouge
        if((*perso)->m_bAlive)
        {
            // Suivant l'action en cours
            switch((*perso)->m_eAction)
            {
            case Engine::ACT_IDLE:
                // Ne fait rien
                (*perso)->update();
                break;
            case Engine::ACT_MOV_LEFT:
            case Engine::ACT_MOV_RIGHT:
            case Engine::ACT_MOV_UP:
            case Engine::ACT_MOV_DOWN:
                // Un déplacement prend 0.5s
                if(((*perso)->m_dBeginAction + 0.5) < getTicks())
                {
                    // S'il est fini, on met à jour la position
                    if((*perso)->m_eAction == Engine::ACT_MOV_LEFT)
                        (*perso)->m_iPosX--;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_RIGHT)
                        (*perso)->m_iPosX++;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_UP)
                        (*perso)->m_iPosY--;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_DOWN)
                        (*perso)->m_iPosY++;

                    // On passe en idle et on relance le script
                    (*perso)->m_eAction = Engine::ACT_IDLE;
                    (*perso)->update();
                }
                break;
            case Engine::ACT_DROP_BOMB:
                // Poser une bombe prend 0.1s
                if(((*perso)->m_dBeginAction + 0.1) < getTicks())
                {
                    // Si c'est fini, on peut bouger à nouveau
                    (*perso)->m_eAction = Engine::ACT_IDLE;
                    (*perso)->update();
                }
                break;
            }
        }

        // On l'unlock
        (*perso)->unlock();
    }

    // Boucle sur les bombes
    std::list<Engine::Bomb*>::iterator bomb = m_Bombs.begin();
    while(bomb != m_Bombs.end())
    {
        // Si elle explose
        if((*bomb)->m_dExplodeDate < getTicks())
        {
            int x = (*bomb)->m_iPosX;
            int y = (*bomb)->m_iPosY;

            m_Explosions[y * m_iWidth + x] = getTicks() + 1.0;

            static const int delta[4][2] = { {1, 0}, {0, -1}, {-1, 0}, {0, 1} };

            // Boucle sur les directions de propagation de l'explosion
            int i;
            for(i = 0; i < 4; i++)
            {
                int x2 = x;
                int y2 = y;
                // On boucle pour parcourir le nombre de cellules prévu
                int c = (*bomb)->m_iCells;
                for(; c > 0; c--)
                {
                    // On se déplace
                    x2 += delta[i][0];
                    y2 += delta[i][1];

                    // Si c'est indestructible, on s'arrête là
                    if(m_Map[y2 * m_iWidth + x2] == Engine::CELL_ROCK)
                        break;

                    // On détruit le terrain
                    m_Map[y2 * m_iWidth + x2] = Engine::CELL_EMPTY;

                    // On fait exploser les bombes
                    std::list<Engine::Bomb*>::iterator it = m_Bombs.begin();
                    for(; it != m_Bombs.end(); it++)
                    {
                        // Explosera le prochain coup
                        (*bomb)->m_dExplodeDate = getTicks();
                    }

                    m_Explosions[y2 * m_iWidth + x2] =
                        getTicks() + 1.0;
                }
            }

            // Supprime la bombe
            std::list<Engine::Bomb*>::iterator temp = bomb;
            bomb++;
            m_Bombs.erase(temp);
        }
        else
            bomb++;
    }

    // Le jeu tourne tant qu'il y a au moins 2 bombermen
    return (nbombers >= 2);
}

void GameEngine::beginOutput()
{
    // On lock tous les personnages
    std::vector<GameEngine::IABomber*>::iterator it =
        m_Bombers.begin();
    for(; it != m_Bombers.end(); it++)
        (*it)->lock();
}

void GameEngine::endOutput()
{
    // On unlock tous les personnages
    std::vector<GameEngine::IABomber*>::iterator it =
        m_Bombers.begin();
    for(; it != m_Bombers.end(); it++)
        (*it)->unlock();
}

const std::vector<Engine::ECell>& GameEngine::getMap() const
{
    return m_Map;
}

std::vector<const Engine::Bomber*> GameEngine::getBombers() const
{
    std::vector<const Engine::Bomber*> bombers;
    bombers.resize(m_Bombers.size());
    bombers.assign(m_Bombers.begin(), m_Bombers.end());
    return bombers;
}

std::vector<const Engine::Bomb*> GameEngine::getBombs() const
{
    std::vector<const Engine::Bomb*> bombs;
    bombs.resize(m_Bombs.size());
    bombs.assign(m_Bombs.begin(), m_Bombs.end());
    return bombs;
}

const std::vector<double>& GameEngine::getExplosions() const
{
    return m_Explosions;
}

GameEngine::IABomber::IABomber(GameEngine *engine, int startx, int starty,
    const char *filename)
  : Engine::Bomber(filename, startx, starty), m_pEngine(engine)
{
}

bool GameEngine::IABomber::move(const std::string &param)
{
    const std::vector<Engine::ECell>& map = m_pEngine->getMap();
    int width = m_pEngine->getMapWidth();
    const int &x = m_iPosX;
    const int &y = m_iPosY;
    int x2, y2;

    Engine::EAction eAction = Engine::ACT_IDLE;

    if(param == "left")
    {
        eAction = Engine::ACT_MOV_LEFT;
        x2 = x-1; y2 = y;
    }
    else if(param == "right")
    {
        eAction = Engine::ACT_MOV_RIGHT;
        x2 = x+1; y2 = y;
    }
    else if(param == "up")
    {
        eAction = Engine::ACT_MOV_UP;
        x2 = x; y2 = y-1;
    }
    else if(param == "down")
    {
        eAction = Engine::ACT_MOV_DOWN;
        x2 = x; y2 = y+1;
    }
    else
        return false;

    // Blocage par le terrain
    if(map[y2 * width + x] != Engine::CELL_EMPTY)
        return false;

    // Blocage par les bombes
    const std::list<Engine::Bomb*> &bombs = m_pEngine->m_Bombs;
    std::list<Engine::Bomb*>::const_iterator it = bombs.begin();
    for(; it != bombs.end(); it++)
    {
        if( ((*it)->m_iPosX == x2) && ((*it)->m_iPosY == y2) )
            return false;
    }

    // On met à jour l'action
    m_eAction = eAction;
    m_dBeginAction = getTicks();

    return true;
}

void GameEngine::IABomber::bomb()
{
    // S'il n'y a pas déjà une bombe à la position indiquée
    bool alreadyABomb = false;
    std::list<Engine::Bomb*> &bombs = m_pEngine->m_Bombs;
    {
        std::list<Engine::Bomb*>::const_iterator it = bombs.begin();
        for(; it != bombs.end(); it++)
        {
            if( ((*it)->m_iPosX == m_iPosX)
             && ((*it)->m_iPosY == m_iPosY) )
            {
                alreadyABomb = true;
                break;
            }
        }
    }
    if(!alreadyABomb)
    {
        // Pose une bombe
        bombs.push_back(new Engine::Bomb(
            m_iPosX, m_iPosY,
            getTicks() + 4.0, 1));
        // TODO : stocker la portée dans Bomber et modifier via les bonus
    }

    m_eAction = Engine::ACT_DROP_BOMB;
    m_dBeginAction = getTicks();
}
