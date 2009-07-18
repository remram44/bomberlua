#include "GameEngine.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <limits>

GameEngine::GameEngine(std::list<std::string> progs)
{
    // Initialize the world (create the map)
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

    // Create the players + load the programs
    m_Bombers.resize(progs.size());
    int i = 0;
    std::list<std::string>::const_iterator it = progs.begin();
#ifdef WITH_PYTHON
    bool py_loaded = false; // Python limitation - only one Python bot
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
            // TODO: max 4 players for now
            std::cerr << "Too many players!\n";
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
                std::cerr << file << ": cannot load more than one Python "
                    "script! (caveat of the Python lib...)\n";
                exit(1);
            }
        }
        else
#endif
        {
            std::cerr << file << ": unrecognized extension\n";
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

    // Loop on players
    std::vector<IABomber*>::iterator perso = m_Bombers.begin();
    for(; perso != m_Bombers.end(); perso++)
    {
        // Lock it
        (*perso)->lock();

        // If this player is not dead
        if((*perso)->m_bAlive)
        {
            nbombers++;

            // Is the player affected by an explosion?
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

        // If he is still not dead, he moves
        if((*perso)->m_bAlive)
        {
            // Depending on the current action
            switch((*perso)->m_eAction)
            {
            case Engine::ACT_IDLE:
                // Don't do anything
                (*perso)->update();
                break;
            case Engine::ACT_MOV_LEFT:
            case Engine::ACT_MOV_RIGHT:
            case Engine::ACT_MOV_UP:
            case Engine::ACT_MOV_DOWN:
                // A move takes 0.5s
                if(((*perso)->m_dBeginAction + 0.5) < getTicks())
                {
                    // If he's done, update the position
                    if((*perso)->m_eAction == Engine::ACT_MOV_LEFT)
                        (*perso)->m_iPosX--;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_RIGHT)
                        (*perso)->m_iPosX++;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_UP)
                        (*perso)->m_iPosY--;
                    else if((*perso)->m_eAction == Engine::ACT_MOV_DOWN)
                        (*perso)->m_iPosY++;

                    // Go to idle and resume the script
                    (*perso)->m_eAction = Engine::ACT_IDLE;
                    (*perso)->update();
                }
                break;
            case Engine::ACT_DROP_BOMB:
                // Planting a bomb takes 0.1s
                if(((*perso)->m_dBeginAction + 0.1) < getTicks())
                {
                    // If it's done, we can move again
                    (*perso)->m_eAction = Engine::ACT_IDLE;
                    (*perso)->update();
                }
                break;
            }
        }

        // Unlock the player
        (*perso)->unlock();
    }

    // Loop on bombs
    std::list<Engine::Bomb*>::iterator bomb = m_Bombs.begin();
    while(bomb != m_Bombs.end())
    {
        // If it's exploding
        if((*bomb)->m_dExplodeDate < getTicks())
        {
            int x = (*bomb)->m_iPosX;
            int y = (*bomb)->m_iPosY;

            m_Explosions[y * m_iWidth + x] = getTicks() + 1.0;

            static const int delta[4][2] = { {1, 0}, {0, -1}, {-1, 0}, {0, 1} };

            // Loop on directions the explosion propagates in
            int i;
            for(i = 0; i < 4; i++)
            {
                int x2 = x;
                int y2 = y;
                // Loop to affect the selected number of cells
                int c = (*bomb)->m_iCells;
                for(; c > 0; c--)
                {
                    // Move
                    x2 += delta[i][0];
                    y2 += delta[i][1];

                    // If indestructible, stop here
                    if(m_Map[y2 * m_iWidth + x2] == Engine::CELL_ROCK)
                        break;

                    // Destroy the wall
                    m_Map[y2 * m_iWidth + x2] = Engine::CELL_EMPTY;

                    // Explode bombs (chain reaction)
                    std::list<Engine::Bomb*>::iterator it = m_Bombs.begin();
                    for(; it != m_Bombs.end(); it++)
                    {
                        // Will explode next time
                        (*bomb)->m_dExplodeDate = getTicks();
                    }

                    m_Explosions[y2 * m_iWidth + x2] =
                        getTicks() + 1.0;
                }
            }

            // Remove the bomb
            std::list<Engine::Bomb*>::iterator temp = bomb;
            bomb++;
            m_Bombs.erase(temp);
        }
        else
            bomb++;
    }

    // The game runs while there are at least 2 bombermen
    return (nbombers >= 2);
}

void GameEngine::beginOutput()
{
    // Lock all the players
    std::vector<GameEngine::IABomber*>::iterator it =
        m_Bombers.begin();
    for(; it != m_Bombers.end(); it++)
        (*it)->lock();
}

void GameEngine::endOutput()
{
    // Unlock all the players
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

    // Blocked by terrain
    if(map[y2 * width + x] != Engine::CELL_EMPTY)
        return false;

    // Blocked by bombs
    const std::list<Engine::Bomb*> &bombs = m_pEngine->m_Bombs;
    std::list<Engine::Bomb*>::const_iterator it = bombs.begin();
    for(; it != bombs.end(); it++)
    {
        if( ((*it)->m_iPosX == x2) && ((*it)->m_iPosY == y2) )
            return false;
    }

    // Update the action
    m_eAction = eAction;
    m_dBeginAction = getTicks();

    return true;
}

void GameEngine::IABomber::bomb()
{
    // If there isn't already a bomb at this location
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
        // Plant a bomb
        bombs.push_back(new Engine::Bomb(
            m_iPosX, m_iPosY,
            getTicks() + 4.0, 1));
        // TODO: store the range in Bomber and change via powerups
    }

    m_eAction = Engine::ACT_DROP_BOMB;
    m_dBeginAction = getTicks();
}
