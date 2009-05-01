#ifndef DISPLAY
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
 * \defgroup display Display module for the application.
 */

/** @{ */
/**
 * Display module.
 *
 * This module handles drawing the current state of the application with SDL.
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
     * Returns a pointer to the unique instance (pattern Singleton).
     */
    static Display *get()
    {
        static Display instance;
        return &instance;
    }

public:
	class InitException : public std::exception
	{
		const char *m_sError;
	public:
		const char *what() 
		{ 
			return m_sError; 
		}
		InitException (const char *s) 
		{ 
			m_sError = s;
		}
	};

	/**
	 * Update the display data and render.
	 */
	static void update(const Engine *engine);

	/**
	 * Initialize the display.
	 */
	static void init(const Engine *engine) throw (InitException);

	/**
	 * Load an image.
	 */
	static void loadImage (SDL_Surface *&surf, const char * path)
		throw (InitException);
};
/** @} */

#endif
