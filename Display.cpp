#include "Display.h"

#include <sstream>

static SDL_Surface *loadImage(const char *path) throw(Display::InitException)
{
    SDL_Surface *surf = IMG_Load(path);
    if(!surf)
    {
        std::ostringstream oss;
        oss << "Error loading: " << path << " : " << IMG_GetError ();
        throw Display::InitException(oss.str());
    }
    return surf;
}

void Display::init(const Engine *engine) throw(InitException)
{
    std::string err;
    if(SDL_Init(SDL_INIT_VIDEO))
    {
        err = "Error initializing SDL: ";
        err += SDL_GetError();
        throw InitException(err);
    }

    if(SDL_SetVideoMode(engine->getMapWidth()*32, engine->getMapHeight()*32, 32,
        SDL_HWSURFACE | SDL_DOUBLEBUF) == NULL)
    {
        err = "Error creating the window: ";
        err += SDL_GetError();
        SDL_Quit();
        throw InitException(err);
    }
    SDL_WM_SetCaption("BomberLua", NULL);

    try
    {
        get()->m_pBrickSurface = loadImage("Images/Cells/Brick.png");
        get()->m_pRockSurface = loadImage("Images/Cells/Rock.png");
        get()->m_pEmptySurface = loadImage("Images/Cells/Empty.png");
        get()->m_pBombermanSurface = loadImage("Images/SFX/Bomberman.png");
        get()->m_pBombSurface = loadImage("Images/SFX/Bomb.png");
        get()->m_pBoomSurface = loadImage("Images/SFX/Wave.png");
    }
    catch(InitException &e)
    {
        SDL_Quit();
        throw e;
    }
}

void Display::update(const Engine *engine)
{
    const std::vector<Engine::ECell> map(engine->getMap());
    Display::get()->drawMapSurface(
        engine->getMapWidth(), engine->getMapHeight(), map);

    std::vector<const Engine::Bomb*> bombs(engine->getBombs());
    Display::get()->drawBombs(bombs);

    std::vector<const Engine::Bomber*> bombers(engine->getBombers());
    Display::get()->drawBombermen(bombers);

    const std::vector<double>& explosions(engine->getExplosions());
    Display::get()->drawExplosions(
        engine->getMapWidth(), engine->getMapHeight(), explosions);

    SDL_Flip(SDL_GetVideoSurface());
}

void Display::drawMapSurface(int width, int height,
    const std::vector<Engine::ECell> &map)
{
    SDL_Rect blitPos;
    blitPos.w = 32;
    blitPos.h = 32;
    blitPos.x = 0;
    blitPos.y = 0;
    SDL_Surface *screen = SDL_GetVideoSurface();
    int x, y;
    for(x = 0; x < width; x++)
    {
        for(y = 0; y < height; y++)
        {
            blitPos.x = x * 32;
            blitPos.y = y * 32;

            switch(map[y * width + x])
            {
            case Engine::CELL_BRICK:
                SDL_BlitSurface(m_pBrickSurface, NULL, screen, &blitPos);
                break;
            case Engine::CELL_ROCK:
                SDL_BlitSurface(m_pRockSurface, NULL, screen, &blitPos);
                break;
            case Engine::CELL_EMPTY:
                SDL_BlitSurface(m_pEmptySurface, NULL, screen, &blitPos);
                break;
            }
        }
    }
}

void Display::drawBombermen(std::vector<const Engine::Bomber*> bombers)
{
    std::vector<const Engine::Bomber*>::iterator it = bombers.begin();
    for(; it != bombers.end(); it++)
    {
        if((*it)->m_bAlive)
        {
            // Look how much time passed since the begining of the action to
            // deduce the position and the frame of the bomber
            double timeElapsed = ((SDL_GetTicks()/1000.0) - (*it)->m_dBeginAction);
            int frameDrawed = (int)(timeElapsed*32);

            SDL_Rect blitPos;
            blitPos.w = 32;
            blitPos.h = 32;
            blitPos.x = (*it)->m_iPosX*32;// Position the bomber on the map
            blitPos.y = (*it)->m_iPosY*32;

            SDL_Rect framePos;
            framePos.w = 32;
            framePos.h = 32;

            // Offset the bomber if it is moving; the frame to draw is selected
            // depending on elapsed time.
            // If it is planting a bomb or is static, we fix it in the cell it's
            // in.
            switch((*it)->m_eAction)
            {
            case Engine::ACT_MOV_LEFT:
                framePos.x = 32*frameDrawed;
                framePos.y = 96;
                blitPos.x -= 2*frameDrawed;
                break;
            case Engine::ACT_MOV_RIGHT:
                framePos.x = 32*frameDrawed;
                framePos.y = 32;
                blitPos.x += 2*frameDrawed;
                break;
            case Engine::ACT_MOV_UP:
                framePos.x = 32*frameDrawed;
                framePos.y = 0;
                blitPos.y -= 2*frameDrawed;
                break;
            case Engine::ACT_MOV_DOWN:
                framePos.x = 32*frameDrawed;
                framePos.y = 64;
                blitPos.y += 2*frameDrawed;
                break;
            default:
                framePos.x = 0;
                framePos.y = 64;
                break;
            }

            // Check that the frame didn't go beyond the image.
            if(framePos.x > 480)
                framePos.x = 480;

            SDL_BlitSurface(m_pBombermanSurface, &framePos,
                SDL_GetVideoSurface(), &blitPos);
        }
    }
}

void Display::drawBombs(std::vector<const Engine::Bomb*> bombs)
{
    std::vector<const Engine::Bomb*>::iterator it = bombs.begin();
    for(; it != bombs.end(); it++)
    {
        // Check the remaining time to get the frame to be drawn
        double remainingTime = ((*it)->m_dExplodeDate - SDL_GetTicks()/1000.0);
        int frameDrawed = (int)(remainingTime*0.75);

        SDL_Rect blitPos;
        blitPos.w = 32;
        blitPos.h = 32;
        blitPos.x = (*it)->m_iPosX*32; // Position the bomber on the map
        blitPos.y = (*it)->m_iPosY*32;

        // By default the bomber is motionless
        SDL_Rect framePos;
        framePos.w = 32;
        framePos.h = 32;
        framePos.x = 32 * frameDrawed;
        framePos.y = 0;

        SDL_BlitSurface(m_pBombSurface, &framePos,
                        SDL_GetVideoSurface(), &blitPos);
    }
}

void Display::drawExplosions(int width, int height,
    const std::vector<double>& explosions)
{
    int x, y;
    double now = SDL_GetTicks()/1000.0;
    for(x = 1; x < width - 1; x++)
    {
        for(y = 1; y < height - 1; y++)
        {
            SDL_Rect blitPos;
            blitPos.x = x*32;
            blitPos.y = y*32;
            blitPos.w = 32;
            blitPos.h = 32;

            SDL_Rect framePos;
            framePos.x = 32;
            framePos.y = 64;
            framePos.w = 32;
            framePos.h = 32;

            // If there's an explosion on the current cell
            if(explosions[y*width+x] >= now)
            {
                // We need to consider adjacent explosions to choose the image
                // to display (to choose framePos.x and framePos.y)
                bool up = (explosions[(y-1)*width+x] >= now);
                bool down = (explosions[(y+1)*width+x] >= now);
                bool left = (explosions[y*width+(x-1)] >= now);
                bool right = (explosions[y*width+(x+1)] >= now);

                if(up && down && left && right)
                {
                    framePos.x = 0;
                    framePos.y = 64;
                }
                else if(up && down && right)
                {
                    framePos.x = 32;
                    framePos.y = 64;
                }
                else if(down && left && right)
                {
                    framePos.x = 64;
                    framePos.y = 64;
                }
                else if(up && down && left)
                {
                    framePos.x = 0;
                    framePos.y = 96;
                }
                else if(up && left && right)
                {
                    framePos.x = 32;
                    framePos.y = 96;
                }
                else if(up && down)
                {
                    framePos.x = 32;
                    framePos.y = 32;
                }
                else if(up)
                {
                    framePos.x = 64;
                    framePos.y = 32;
                }
                else if(down)
                {
                    framePos.x = 0;
                    framePos.y = 32;
                }
                else if(left && right)
                {
                    framePos.x = 32;
                    framePos.y = 0;
                }
                else if(left)
                {
                    framePos.x = 64;
                    framePos.y = 0;
                }
                else if(right)
                {
                    framePos.x = 0;
                    framePos.y = 0;
                }
                SDL_BlitSurface(m_pBoomSurface, &framePos,
                    SDL_GetVideoSurface(), &blitPos);
            }
        }
    }
}

Display::~Display()
{
    SDL_FreeSurface(m_pBrickSurface);
    SDL_FreeSurface(m_pRockSurface);
    SDL_FreeSurface(m_pEmptySurface);
    SDL_FreeSurface(m_pBombermanSurface);
    SDL_FreeSurface(m_pBombSurface);
    SDL_FreeSurface(m_pBoomSurface);
    SDL_Quit();
}
