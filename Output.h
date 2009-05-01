#ifndef OUTPUT_H
#define OUTPUT_H

#include "BomberLua.h"
#include "Socket.h"
#include "Display.h"

#include <SDL/SDL.h>

/**
 * \defgroup output Module de sortie du programme
 */
/** @{ */
/**
 * Module de sortie.
 *
 * Ce module se charge de la sortie du programme, en affichant les données via
 * SDL et/ou en les envoyant par le réseau.
 */
class Output {

private:
    /** Indique si on agit en tant que serveur, en envoyant les données aux
     * clients qui se connecte à nous. */
    bool m_bServer;

    /** Indique si on est graphique, donc si on affiche localement les
     * informations. */
    bool m_bGraphic;

    /** La socket, placée en écoute si m_bServer est true. */
    ServerSocket m_Socket;

private:
    /**
     * Retourne un pointeur sur l'instance (modèle Singleton).
     */
    static Output *get()
    {
        static Output instance;
        return &instance;
    }

public:
    /**
     * Initialise le module, via le mode de fonctionnement du programme
     * sélectionné.
     */
    static void init(bool graphic, int server, const Engine *engine);

    /**
     * Met à jour la sortie, en acceptant les connexions, envoyant les données
     * et/ou affichant avec SDL.
     */
    static bool update(const Engine *engine);
	
};
/** @} */

#endif
