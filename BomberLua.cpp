#include "BomberLua.h"

#include "Output.h"

#include "NetworkReceiver.h"
#include "GameEngine.h"

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <limits>

// Affiche l'aide
static void help(std::ostream& out)
{
    out << "bomberlua [options] [programmes]\n"
        "Les options reconnues sont :\n"
        "  -h ou --help : affiche cette aide et quitte"
        "  -i ou --graphic : active l'affichage graphique du jeu (avec SDL)\n"
        "Exemple : bomberlua -i bots/test.lua bots/test.lua\n";
}

BomberLua::BomberLua()
  : m_bRunning(true)
{
}

int BomberLua::launch(int argc, char **argv)
{
    // Initialise
    int ret = get()->init(argc, argv);

    if(ret == 1)
        return 1;
    if(ret == 2)
        return 0;
    
    // Puis lance le jeu
    return get()->run();
}

int BomberLua::init(int argc, char **argv)
{
    bool graphic = false;  // affichage local ?

    int server = -1;   // accepter les connexions sur ce port

    bool client = false;   // recevoir les données par le réseau ?
    const char *addr = NULL;
    int port = -1;

    // Les programmes à charger
    std::list<std::string> progs;

    int i;
    for(i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);

        // Les options commencent par un -
        if(argv[i][0] == '-')
        {
            if(arg == "-h" || arg == "--help")
            {
                help(std::cout);
                return 2;
            }
            else if(arg == "-i" || arg == "--graphic")
                graphic = true;
            else if(arg == "-s" || arg == "--server")
            {
                i++;
                std::istringstream iss(argv[i]);
                iss >> server;
                if(iss.fail() || !iss.eof()
                 || (server < 1) || (server > 65535) )
                {
                    std::cerr << "Numéro de port invalide\n";
                    return 1;
                }
            }
            else if(arg == "-c" || arg == "--client")
            {
                client = true;
                i++;
                addr = argv[i];
                i++;
                std::istringstream iss(argv[i]);
                iss >> port;
                if(iss.fail() || !iss.eof())
                {
                    std::cerr << "Numéro de port invalide\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Option \"" << arg << "\" non-reconnue\n";
                std::cerr << "Utilisez --help pour l'aide\n";
                return 1;
            }
        }

        // Si ce n'est pas une option, c'est un programme à charger
        else
            progs.push_back(arg);
    }

    if(progs.size() < 1)
    {
        std::cerr << "Vous devez indiquer au moins un programme à "
            "charger !\n";
        return 1;
    }

    // Créé le moteur
    if(client) // soit un moteur récupérant les données depuis un serveur
        m_pEngine = new NetworkReceiver(addr, port);
    else // soit le véritable moteur du jeu qui va générer les informations
        m_pEngine = new GameEngine(progs);
	
	Output::init(graphic, server, m_pEngine); // initialise le module de rendu
	
    return 0;
}

int BomberLua::run()
{
    while(m_bRunning)
    {
        // On se met à jour
        if(!m_pEngine->update())
            m_bRunning = false;

        // On affiche les informations
        m_pEngine->beginOutput();
        if(!Output::update(m_pEngine))
            m_bRunning = false;
        m_pEngine->endOutput();

        // On dort 0.05 secondes
        SDL_Delay(50);
    }

    // Affiche le résultat sur la sortie
    std::cout << "Fin de la partie\n";

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
        std::cerr << "Gagnant : " << winner->m_sFilename << "\n";

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
