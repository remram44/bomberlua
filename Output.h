#ifndef OUTPUT_H
#define OUTPUT_H

#include "BomberLua.h"

#ifndef _NO_GRAPHICS
#include "Display.h"
#include <SDL/SDL.h>
#endif

/**
 * \defgroup output Output module for the application.
 */
/** @{ */
/**
 * Output module.
 *
 * This module handles the output of the application, by displaying the data
 * through SDL and/or sending them via the network.
 */
class Output {

private:
    /** Indicates whether we are using graphical output, displaying the data
     * informations. */
    bool m_bGraphic;

private:
    /**
     * Return a pointer to the unique instance (pattern Singleton).
     */
    static Output *get()
    {
        static Output instance;
        return &instance;
    }

public:
    /**
     * Initialize the module, via the selected application mode.
     */
    static void init(bool graphic, const Engine *engine);

    /**
     * Update the output, by accepting connections, sending data, and/or
     * displaying using SDL.
     */
    static bool update(const Engine *engine);
    
};
/** @} */

#endif
