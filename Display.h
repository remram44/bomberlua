#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <exception>
#include "GameEngine.h"

/**
 * \ingroup output
 */

/**
 * \defgroup display Module d'affichage du programme.
 */

/** @{ */
/**
 * Module d'affichage.
 *
 * Ce module se charge d'afficher via la SDL l'état actuel du programme.
 */
class Display {

private:
    SDL_Surface *m_pBrickSurface;
    SDL_Surface *m_pRockSurface;
    SDL_Surface *m_pEmptySurface;
    SDL_Surface *m_pBombermanSurface;
    SDL_Surface *m_pBombSurface;
    SDL_Surface *m_pBoomSurface;

    void drawMapSurface(int width, int height,
        const std::vector<Engine::ECell> &map);
    void drawBombermen(const std::vector<const Engine::Bomber*> bombers);
    void drawBombs(const std::vector<const Engine::Bomb*> bombs);
    void drawExplosions(int width, int height, const std::vector<double>& explosions);
    ~Display();

private:
    /**
     * Retourne un pointeur sur l'instance (modèle Singleton).
     */
    static Display *get()
    {
        static Display instance;
        return &instance;
    }

public:
    class InitException {

    private:
        std::string m_sError;

    public:
        const char *what()
        {
            return m_sError.c_str();
        }
        InitException(std::string s)
        {
            m_sError = s;
        }

    };

    /**
     * Met a jour les données d'affichage et affiche.
     */
    static void update(const Engine *engine);

    /**
     * Initialisation de l'affichage.
     */
    static void init(const Engine *engine) throw(InitException);

};
/** @} */

#endif
