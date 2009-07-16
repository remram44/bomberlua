#include "BomberLua.h"
#include <SDL/SDL.h>
#include <cstdlib>
#include <ctime>

/**
 * \mainpage BomberLua
 *
 * \section Introduction
 * %BomberLua is a game project, based on Bomberman, where the players are not
 * controller directly by humans but by AIs. An AI is a Lua script that controls
 * a player.
 *
 * \section Rules
 * Currently, the game field is fixed (11x11), made out of a grid of ROCK cells
 * and filled with BRICK cells, except in the 4 corner, where the bombermen
 * start.
 *
 * Only the BRICK cells are destructible. The bombs have a fixed range of 1 cell
 * if each direction.
 *
 * The scripts are executed in parallel in separate threads. They are loaded
 * before the game starts and the bombermen are placed; however they have
 * access to the map. This time can be used to precompute some information, for
 * instance pathfind data (although for now, the size of maps don't warrant it).
 * The ready() function must be called before the 5 seconds end; it will return
 * (and allow the script to continue) at the start of the game. At that point
 * all the functions will be available.
 *
 * The functions allowing the script to act are blockingm meaning that they only
 * return once the action is over and the bomberman can act again. The duration
 * of actions is:
 *   - 0.5 seconds for a move (move() function)
 *   - 0.1 seconds to plant a bomb (bomb() function)
 *
 * \section Links
 * The official page of the project, with more informations and links to the
 * latest versions is here: http://wiki.nyug.org/bomberlua
 *
 * Bugtracker : http://remram44.free.fr/hyper/bugdar/
 */

int main(int argc, char **argv)
{
    srand(time(NULL));
    return BomberLua::launch(argc, argv);
}
