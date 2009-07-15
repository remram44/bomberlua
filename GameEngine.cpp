#include "GameEngine.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

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
                + (*perso)->m_iPosX] > (SDL_GetTicks()/1000.0);

            if(!bExplode)
                switch((*perso)->m_eAction)
                {
                case Engine::ACT_MOV_LEFT:
                    bExplode = m_Explosions[(*perso)->m_iPosY * m_iWidth
                        + ( (*perso)->m_iPosX - 1)] > (SDL_GetTicks()/1000.0);
                    break;
                case Engine::ACT_MOV_UP:
                    bExplode = m_Explosions[( (*perso)->m_iPosY - 1) * m_iWidth
                        + (*perso)->m_iPosX] > (SDL_GetTicks()/1000.0);
                    break;
                case Engine::ACT_MOV_RIGHT:
                    bExplode = m_Explosions[(*perso)->m_iPosY * m_iWidth
                        + ( (*perso)->m_iPosX + 1)] > (SDL_GetTicks()/1000.0);
                    break;
                case Engine::ACT_MOV_DOWN:
                    bExplode = m_Explosions[( (*perso)->m_iPosY + 1) * m_iWidth
                        + (*perso)->m_iPosX] > (SDL_GetTicks()/1000.0);
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
                // S'il ne fait rien depuis plus de 0.5s, on considère qu'il a
                // planté
                if((*perso)->m_dBeginAction + 0.5 < (SDL_GetTicks()/1000.0))
                {
                    std::cerr << "timeout : killing script\n";
                    (*perso)->kill();
                }
                (*perso)->update();
                break;
            case Engine::ACT_MOV_LEFT:
            case Engine::ACT_MOV_RIGHT:
            case Engine::ACT_MOV_UP:
            case Engine::ACT_MOV_DOWN:
                // Un déplacement prend 0.5s
                if(((*perso)->m_dBeginAction + 0.5) < (SDL_GetTicks()/1000.0))
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
                if(((*perso)->m_dBeginAction + 0.1) < (SDL_GetTicks()/1000.0))
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
        if((*bomb)->m_dExplodeDate < (SDL_GetTicks()/1000.0))
        {
            int x = (*bomb)->m_iPosX;
            int y = (*bomb)->m_iPosY;

            m_Explosions[y * m_iWidth + x] =
                (SDL_GetTicks()/1000.0) + 1.0;

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
                        (*bomb)->m_dExplodeDate = SDL_GetTicks()/1000.0;
                    }

                    m_Explosions[y2 * m_iWidth + x2] =
                        (SDL_GetTicks()/1000.0) + 1.0;
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

void GameEngine::IABomber::wait()
{
    // On indique que le script a agit
    m_dBeginAction = SDL_GetTicks()/1000.0;
}

bool GameEngine::IABomber::move(const std::string &param)
{
    const std::vector<Engine::ECell>& map = m_pEngine->getMap();
    int width = m_pEngine->getMapWidth();
    const int &x = m_iPosX;
    const int &y = m_iPosY;

    Engine::EAction eAction = Engine::ACT_IDLE;

    // TODO : Les bombes bloquent le passage !
    if( (param == "left") && (map[y * width + (x - 1)]
        == Engine::CELL_EMPTY) )
    {
        eAction = Engine::ACT_MOV_LEFT;
    }
    else if( (param == "right") && (map[y * width + (x + 1)]
    == Engine::CELL_EMPTY) )
    {
        eAction = Engine::ACT_MOV_RIGHT;
    }
    else if( (param == "up") && (map[(y - 1) * width + x]
        == Engine::CELL_EMPTY) )
    {
        eAction = Engine::ACT_MOV_UP;
    }
    else if( (param == "down") && (map[(y + 1) * width + x]
        == Engine::CELL_EMPTY) )
    {
        eAction = Engine::ACT_MOV_DOWN;
    }
    else if( (param != "left") && (param != "right")
     && (param != "up") && (param != "down") )
        return false;

    // On met à jour l'action
    m_eAction = eAction;
    m_dBeginAction = SDL_GetTicks()/1000.0;

    return true;
}

void GameEngine::IABomber::bomb()
{
    // S'il n'y a pas déjà une bombe à la position indiquée
    bool alreadyABomb = false;
    {
        std::vector<const Engine::Bomb*> bombs = m_pEngine->getBombs();
        std::vector<const Engine::Bomb*>::const_iterator it = bombs.begin();
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
        m_pEngine->m_Bombs.push_back(new Engine::Bomb(
            m_iPosX, m_iPosY,
            SDL_GetTicks()/1000.0 + 4.0, 1));
        // TODO : stocker dans Bomber et modifier via les bonus
    }

    m_eAction = Engine::ACT_DROP_BOMB;
    m_dBeginAction = SDL_GetTicks()/1000.0;
}
