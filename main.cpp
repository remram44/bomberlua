#include "BomberLua.h"
#include <SDL/SDL.h>
#include <cstdlib>
#include <ctime>

/**
 * \mainpage BomberLua
 * \section Introduction
 * %BomberLua is a game project, based on Bomberman, where the players are not
 * controller directly by humans but by AIs. An AI is a Lua script that controls
 * a player.
 */

int main(int argc, char **argv)
{
    srand(time(NULL));
    return BomberLua::launch(argc, argv);
}
