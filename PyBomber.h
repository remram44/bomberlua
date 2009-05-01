#ifndef PYBOMBER_H
#define PYBOMBER_H

#include "GameEngine.h"

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <Python.h>

/**
 * Un joueur controllé par un script Python.
 */
class PyBomber : public GameEngine::IABomber {

private:
    /** Fonctions python, voir LuaBomber.h pour les descriptions */
    static PyObject *py_log(PyObject *, PyObject *);
    static PyObject *py_ready(PyObject *, PyObject *);
    static PyObject *py_wait(PyObject *, PyObject *);
    static PyObject *py_move(PyObject *, PyObject *);
    static PyObject *py_bomb(PyObject *, PyObject *);
    static PyObject *py_get_self(PyObject *, PyObject *);
    static PyObject *py_get_bombers(PyObject *, PyObject *);
    static PyObject *py_get_field(PyObject *, PyObject *);
private:
    static int run(void *data);
private:
    /** Thread de l'IA. */
    SDL_Thread *m_Thread;

    /** Condition utilisée pour indiquer la fin de l'exécution du script. */
    SDL_cond *m_CondScriptActed;

    /** Condition utilisée pour provoquer la reprise de l'exécution du
     * script. */
    SDL_cond *m_CondScriptResume;

    /** Mutex */
    SDL_mutex *m_Mutex;

public:
    std::string m_Filename;
    /**
     * Constructeur.
     *
     * Compile le script Python.
     * @param filename Nom du fichier Python à utiliser.
     */
    PyBomber(GameEngine *engine, int startx, int starty,
        const char *filename);

    /**
     * Destructeur.
     */
    ~PyBomber();

    /**
     * Reprend l'exécution du script dans un thread.
     */
    void update();

    /**
     * Tue ce personnage.
     *
     * Il n'est alors plus vivant, et le thread est tué s'il était actif.
     */
    void kill();

    /**
     * Verrouille le personnage.
     *
     * Tout accès aux membres du personnage nécessite son verrouillage, car ils
     * sont succeptibles d'être utilisés également depuis les threads.
     */
    void lock();

    /**
     * Déverrouille le personnage.
     */
    void unlock();


};

#endif
