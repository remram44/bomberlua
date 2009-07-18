#include "Display.h"

#include <sstream>

static SDL_Surface *loadImage(const char *path) throw(Display::InitException)
{
    SDL_Surface *surf = IMG_Load(path);
    if(!surf)
    {
        std::ostringstream oss;
        oss << "Erreur au chargement de : " << path << " : " << IMG_GetError();
        throw Display::InitException(oss.str());
    }
    return surf;
}

void Display::init(const Engine *engine) throw(InitException)
{
    std::string err;
    if(SDL_Init(SDL_INIT_VIDEO))
    {
        err = "Erreur à l'initialisation de SDL : ";
        err += SDL_GetError();
        throw InitException(err);
    }

    if(SDL_SetVideoMode(engine->getMapWidth()*32, engine->getMapHeight()*32, 32,
        SDL_HWSURFACE | SDL_DOUBLEBUF) == NULL)
    {
        err = "Erreur à la création de la fenêtre : ";
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
            // On regarde le temps écoulé depuis le début de l'action pour en
            // déduire la position et la frame du bomber
            double timeElapsed = ((SDL_GetTicks()/1000.0) - (*it)->m_dBeginAction);
            int frameDrawed = (int)(timeElapsed*32);

            SDL_Rect blitPos;
            blitPos.w = 32;
            blitPos.h = 32;
            blitPos.x = (*it)->m_iPosX*32; // On place le bomber sur le terrain
            blitPos.y = (*it)->m_iPosY*32;

            SDL_Rect framePos;
            framePos.w = 32;
            framePos.h = 32;

            // On décale le bomber si il est en mouvement ; la frame à dessiner
            // est choisie via le temps écoulé.
            // Si il pose une bombe ou si il est immobile, on le fixe sur la
            // case ou il est.
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

            // On vérifie que la frame n'a pas dépassé l'image.
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
        // On regarde le temps restant pour en déduire la frame affichée
        double remainingTime = ((*it)->m_dExplodeDate - SDL_GetTicks()/1000.0);
        int frameDrawed = (int)(remainingTime*0.75);

        SDL_Rect blitPos;
        blitPos.w = 32;
        blitPos.h = 32;
        blitPos.x = (*it)->m_iPosX*32; // On place le bomber sur le terrain
        blitPos.y = (*it)->m_iPosY*32;

        // Par défaut le bomber est immobile
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

            // S'il y a une explosion sur la case actuelle
            if(explosions[y*width+x] >= now)
            {
                // On doit regarder les explosions alentour pour déterminer
                // l'image à afficher (donc choisir framePos.x et framePos.y)
                int adjacent = 0;
                if(explosions[(y-1)*width+x] >= now) adjacent++;
                if(explosions[(y+1)*width+x] >= now) adjacent++;
                if(explosions[y*width+(x-1)] >= now) adjacent++;
                if(explosions[y*width+(x+1)] >= now) adjacent++;

                if(adjacent >= 3)
                {
                    framePos.x = 0;
                    framePos.y = 64;
                }
                else if( (explosions[(y-1)*width+x] >= now)
                      && (explosions[(y+1)*width+x] >= now) )
                {
                    framePos.x = 32;
                    framePos.y = 32;
                }
                else if(explosions[(y-1)*width+x] >= now)
                {
                    framePos.x = 64;
                    framePos.y = 32;
                }
                else if(explosions[(y+1)*width+x] >= now)
                {
                    framePos.x = 0;
                    framePos.y = 32;
                }
                else if( (explosions[y*width+(x-1)] >= now)
                      && (explosions[y*width+(x+1)] >= now) )
                {
                    framePos.x = 32;
                    framePos.y = 0;
                }
                else if(explosions[y*width+x-1] >= now)
                {
                    framePos.x = 64;
                    framePos.y = 0;
                }
                else if(explosions[y*width+x+1] >= now)
                {
                    framePos.x = 0;
                    framePos.y = 0;
                }
                SDL_BlitSurface(m_pBoomSurface, &framePos, SDL_GetVideoSurface(), &blitPos);
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
