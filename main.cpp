#include "BomberLua.h"
#include <SDL/SDL.h>
#include <cstdlib>
#include <ctime>

/**
 * \mainpage BomberLua
 *
 * \section Introduction
 * %BomberLua est un projet de jeu, basé sur Bomberman, où les personnages ne
 * sont plus contrôlés par les joueurs mais par des IAs. Une IA est en fait un
 * script Lua qui contrôle les déplacements du personnage.
 *
 * \section Règles
 * Actuellement, le terrain de jeu est fixe (11×11), composé d'une grille de
 * blocs ROCK et complété par des blocs BRICK, sauf dans les 4 coins, où les
 * bombermen commencent.
 *
 * Seuls les blocs BRICK sont destructibles. Les bombes ont une portée fixe de 1
 * bloc dans chaque direction.
 *
 * Les scripts sont exécutés en parallèle dans des threads séparés. Ils sont
 * chargés avant le début de la partie et le placement des bombermen ; ils ont
 * cependant accès à l'aire de jeu. Cette période peut être utilisée pour
 * précalculer certaines informations, par exemple concernant le pathfinding
 * (bien que pour l'instant, la taille des maps ne le justifie pas). La fonction
 * ready() doit être appelée avant la fin des 5 secondes ; elle retournera (et
 * rendra donc la main au script) au début de la partie ; à ce moment toutes les
 * fonctions deviennent disponibles.
 *
 * Les fonctions permettant au script de réaliser des actions sont bloquantes,
 * c'est à dire qu'elles ne retournent que lorsque l'action est terminée et que
 * le bomberman peut à nouveau entreprendre une action. La durée de ces actions
 * est de :
 *   - 0.5 secondes pour un mouvement (fonction move()) ;
 *   - 0.1 secondes pour poser une bombe (fonction bomb()) ;
 *
 * \section Liens
 * La page officielle du projet, avec plus d'informations et les liens vers les
 * dernières versions est accessible ici : http://wiki.nyug.org/bomberlua
 *
 * Bugtracker : http://remram44.free.fr/hyper/bugdar/
 */

int main(int argc, char **argv)
{
    srand(time(NULL));
    return BomberLua::launch(argc, argv);
}
