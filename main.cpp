#include "BomberLua.h"
#include <SDL/SDL.h>
#include <cstdlib>
#include <ctime>

/**
 * \mainpage BomberLua
 * \section Introduction
 * %BomberLua est un projet de jeu, basé sur Bomberman, où les personnages ne
 * sont plus contrôlés par les joueurs mais par des IAs. Une IA est en fait un
 * script Lua qui contrôle les déplacements du personnage.
 */

int main(int argc, char **argv)
{
    srand(time(NULL));
    return BomberLua::launch(argc, argv);
}
