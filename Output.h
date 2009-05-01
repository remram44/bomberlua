#ifndef OUTPUT_H
#define OUTPUT_H

#include "BomberLua.h"
#include "Socket.h"
#include "Display.h"

#include <SDL/SDL.h>

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
    /** Indicates that we are acting as a server, sending data to clients that
     * connect to us. */
    bool m_bServer;

    /** Indicates whether we are using graphical output, displaying the data
     * locally. */
    bool m_bGraphic;

    /** The socket, listening if m_bServer is true. */
    ServerSocket m_Socket;

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
    static void init(bool graphic, int server, const Engine *engine);

    /**
     * Update the output, by accepting connections, sending data, and/or
     * displaying using SDL.
     */
    static bool update(const Engine *engine);
	
};
/** @} */

#endif
