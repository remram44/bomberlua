#include "BomberLua.h"

#include "Output.h"

#include "GameEngine.h"

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <limits>

#ifdef _TICKS_USE_SDL
#ifndef _NO_GRAPHICS
#include <SDL/SDL.h>
#else
#error _TICKS_USE_SDL defined but _NO_GRAPHICS set!
#endif
#else
#include <sys/time.h>
#include <unistd.h>
#endif

// Get elapsed time since the program started
// Like SDL_GetTicks()/1000.0 but doesn't require SDL
#ifndef _TICKS_USE_SDL
double getTicks()
{
    static int init_sec = -1;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if(init_sec == -1)
        init_sec = tv.tv_sec - 1; // We don't want accuracy, just precision...
    return tv.tv_sec - init_sec + 1000000.0 * tv.tv_usec;
}
#else
double getTicks()
{
    return SDL_GetTicks()/1000.0;
}
#endif

// Show help
static void help(std::ostream& out)
{
    out << "bomberlua [options] [programs]\n"
        "Recognized options are::\n"
        "  -h or --help: show this help and exit"
        "  -i or --graphic: activate graphic display of the match (with SDL)"
#ifdef _NO_GRAPHICS
        " (disabled)"
#endif
        "\nExample: bomberlua -i bots/test.lua bots/test.lua\n";
}

BomberLua::BomberLua()
  : m_bRunning(true)
{
}

int BomberLua::launch(int argc, char **argv)
{
    // Initialize
    int ret = get()->init(argc, argv);

    if(ret == 1)
        return 1;
    if(ret == 2)
        return 0;

    // Then start the game
    return get()->run();
}

int BomberLua::init(int argc, char **argv)
{
    bool graphic = false;  // local display?

    // Programs to load
    std::list<std::string> progs;

    int i;
    for(i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        // Options begin with a -
        if(argv[i][0] == '-')
        {
            if(arg == "-h" || arg == "--help")
            {
                help(std::cout);
                return 2;
            }
            else if(arg == "-i" || arg == "--graphic")
            {
                #ifdef _NO_GRAPHICS
                    std::cerr << " Option \"-i\": graphic display is disabled. "
                        "Recompile the application if you want to use it.\n";
                    return 1;
                #endif
                graphic = true;
            }
            else
            {
                std::cerr << "Unrecognized option \"" << arg << "\"\n";
                std::cerr << "Use --help for help\n";
                return 1;
            }
        }

        // If it is not an option, it's a program to be loaded
        else
            progs.push_back(arg);
    }

    if(progs.size() < 1)
    {
        std::cerr << "You need to specify at least one program to load!\n";
        return 1;
    }

    // Create the engine
    m_pEngine = new GameEngine(progs);
    // TODO: ReplayEngine: engine replaying a recorded game

    Output::init(graphic, m_pEngine); // initialize the rendering module

    return 0;
}

int BomberLua::run()
{
    while(m_bRunning)
    {
        // Update
        if(!m_pEngine->update())
            m_bRunning = false;

        // Display the data
        m_pEngine->beginOutput();
        if(!Output::update(m_pEngine))
            m_bRunning = false;
        m_pEngine->endOutput();

        // Sleep 0.05 seconds
#ifndef _NO_GRAPHICS
        SDL_Delay(50);
#endif
    }

    // Display the results on the standard output
    std::cout << "Match ended\n";

    std::vector<const Engine::Bomber*> bombers = m_pEngine->getBombers();
    const Engine::Bomber *winner = NULL;
    size_t i;
    for(i = 0; i < bombers.size(); i++)
    {
        if(bombers[i]->m_bAlive)
        {
            if(winner == NULL)
                winner = bombers[i];
            else
            {
                winner = NULL;
                break;
            }
        }
    }
    if(winner != NULL)
        std::cerr << "Winner: " << winner->m_sFilename << "\n";

    return 0;
}

BomberLua::~BomberLua()
{
    delete m_pEngine;
}

Engine::~Engine()
{
}

void Engine::beginOutput()
{
}

void Engine::endOutput()
{
}

Engine::Bomber::Bomber(const char *filename, int posX, int posY)
  : m_sFilename(filename), m_iPosX(posX), m_iPosY(posY),
    m_bAlive(true), m_eAction(Engine::ACT_IDLE)
{
    m_dBeginAction = std::numeric_limits<double>::infinity();
}

Engine::Bomb::Bomb(int posX, int posY, double explodeDate, int cells)
  : m_iPosX(posX), m_iPosY(posY),
    m_dExplodeDate(explodeDate), m_iCells(cells)
{
}
