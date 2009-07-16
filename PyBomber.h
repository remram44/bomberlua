#ifndef PYBOMBER_H
#define PYBOMBER_H

#include "GameEngine.h"

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <Python.h>

/**
 * A player controller from a Python script.
 */
class PyBomber : public GameEngine::IABomber {

private:
    /** Python functions, see LuaBomber.h for descriptions */
    static PyObject *py_log(PyObject *, PyObject *);
    static PyObject *py_ready(PyObject *, PyObject *);
    static PyObject *py_move(PyObject *, PyObject *);
    static PyObject *py_bomb(PyObject *, PyObject *);
    static PyObject *py_get_self(PyObject *, PyObject *);
    static PyObject *py_get_bombers(PyObject *, PyObject *);
    static PyObject *py_get_field(PyObject *, PyObject *);
private:
    static int run(void *data);
private:
    /** AI's thread. */
    SDL_Thread *m_Thread;

    /** Condition used to indicate the end of a script's execution. */
    SDL_cond *m_CondScriptActed;

    /** Condition used to make the script resume execution. */
    SDL_cond *m_CondScriptResume;

    /** Mutex */
    SDL_mutex *m_Mutex;

public:
    std::string m_Filename;
    /**
     * Constructor.
     *
     * Compiles the Python script.
     * @param filename Name of the file to use.
     */
    PyBomber(GameEngine *engine, int startx, int starty,
        const char *filename);

    /**
     * Destructor.
     */
    ~PyBomber();

    /**
     * Resume execution of the script in a thread.
     */
    void update();

    /**
     * Kill this player.
     *
     * He is then no longer alive, and the thread is killed if it was running.
     */
    void kill();

    /**
     * Lock this player.
     *
     * Accessing attributes of the player requires it to be locked, since they
     * can be accessed from threads.
     */
    void lock();

    /**
     * Unlock the player.
     */
    void unlock();


};

#endif
