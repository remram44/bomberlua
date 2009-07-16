#ifdef WITH_PYTHON
#include "PyBomber.h"

#include <iostream>
#include <sstream>

PyObject *PyBomber::py_log(PyObject *self, PyObject *args)
{
    const char *text;
    if (!PyArg_ParseTuple(args, "s", &text))
        return NULL;
    std::cerr << "Log: " << text << "\n";
    Py_INCREF(Py_None);
    return Py_None;
}
PyObject *PyBomber::py_ready(PyObject *self, PyObject *args)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    bomber->lock();

    // Mark that the script has acted
    SDL_CondSignal(bomber->m_CondScriptActed);

    // Wait for a signal to resume execution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyBomber::py_move(PyObject *self, PyObject *args)
{
    char *direction;
    std::string param;
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    if (!PyArg_ParseTuple(args, "s", &direction))
        return NULL;
    param = direction;
    bomber->lock();
    if(!bomber->move(param))
        return NULL;

    // Mark that the script has acted
    SDL_CondSignal(bomber->m_CondScriptActed);

    // Wait for a signal to resume execution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyBomber::py_bomb(PyObject *self, PyObject *args)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    bomber->lock();

    bomber->bomb();

    // Mark that the script has acted
    SDL_CondSignal(bomber->m_CondScriptActed);

    // Wait for a signal to resume execution
    SDL_CondWait(bomber->m_CondScriptResume, bomber->m_Mutex);

    bomber->unlock();

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyBomber::py_get_self(PyObject *self, PyObject *args)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    PyObject *ret = Py_BuildValue("(i,i)", bomber->m_iPosX, bomber->m_iPosY);
    return ret;
}

PyObject *PyBomber::py_get_bombers(PyObject *self, PyObject *args)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    std::vector<const Engine::Bomber*> bombers =
        bomber->m_pEngine->getBombers();
    int i;
    int n = bombers.size();
    PyObject *list;
    list = PyList_New(0);
    for (i = 0; i<n; i++)
    {
        const Engine::Bomber *it = bombers[i];
        if( !(it == bomber) && (it->m_bAlive) )
        {
            // Create the positions of the bomber
            PyList_Append(list,
                Py_BuildValue("(i,i)", it->m_iPosX, it->m_iPosY));
        }
        // Add them to the list
    }
    std::cerr << "\n";
    return list;
}

PyObject *PyBomber::py_get_field(PyObject *self, PyObject *args)
{
    // Get a pointer to the associated Bomber
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    GameEngine *engine = bomber->m_pEngine;

    // Create the dictionary
    // it will be of the form:
    // {"w":2, "h":2, (0,0):"empty" ...}
    PyObject *ret = PyDict_New();
    // Write the size
    int width = engine->getMapWidth();
    int height = engine->getMapHeight();
    PyDict_SetItemString(ret, "w", Py_BuildValue("i", width));
    PyDict_SetItemString(ret, "h", Py_BuildValue("i", height));


    // Then loop
    const std::vector<Engine::ECell> &map = engine->getMap();
    std::vector<const Engine::Bomb*> bombs = engine->getBombs();
    const std::vector<double> &explosions = engine->getExplosions();
    int x, y;
    PyObject *type = NULL;
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < height; x++)
        {
            switch(map[y * width + x])
            {
            // Indestructible wall
            case Engine::CELL_ROCK:
                type = Py_BuildValue("s", "rock");
                break;
            // Destructible wall
            case Engine::CELL_BRICK:
                type = Py_BuildValue("s", "brick");
                break;
            // No wall; perhaps a bomb or an explosion?
            case Engine::CELL_EMPTY:
                // An explosion
                if(explosions[y * width + x]
                 >= (SDL_GetTicks()/1000.0) )
                    type = Py_BuildValue("s", "explosion");
                else
                {
                    // A bomb
                    std::vector<const Engine::Bomb*>::const_iterator bomb;
                    bomb = bombs.begin();
                    for(; bomb != bombs.end(); bomb++)
                    {
                        if( ((*bomb)->m_iPosX == x) && ((*bomb)->m_iPosY == y) )
                        {
                            type = Py_BuildValue("f", (*bomb)->m_dExplodeDate
                                 - (SDL_GetTicks()/1000.0));
                            break;
                        }
                    }
                    if(bomb == bombs.end())
                        type = Py_BuildValue("s", "empty");
                }
                break;

            }
            PyDict_SetItem(ret, Py_BuildValue("(i,i)", x, y), type);
        }
    }

    return ret;
}
PyBomber::PyBomber(GameEngine *engine, int startx, int starty,
    const char *filename)
  : GameEngine::IABomber(engine, startx, starty, filename),
    m_Thread(NULL),
    m_CondScriptActed(NULL), m_CondScriptResume(NULL), m_Mutex(NULL),
    m_Filename(filename)
{
    // Create the condition used to signal the script is done thinking
    m_CondScriptActed = SDL_CreateCond();

    // Create the condition to signal the engine is waiting for a new action,
    // and that the script should resume execution
    m_CondScriptResume = SDL_CreateCond();

    // Create the mutex
    m_Mutex = SDL_CreateMutex();

    // Load python
    static PyMethodDef methods[] = {
        {"log", py_log, METH_VARARGS, "Logs information"},
        {"ready", py_ready, METH_VARARGS,
            "Indicates that the script is initialized"},
        {"move", py_move, METH_VARARGS, "Move around. The possible parameters "
            "are \"left\", \"right\", \"up\", \"down\""},
        {"bomb", py_bomb, METH_VARARGS, "Plant a bomb"},
        {"get_self", py_get_self, METH_VARARGS,
            "Return the players's position, as a tuple"},
        {"get_bombers", py_get_bombers, METH_VARARGS, "Return the other "
            "players' positions as a list of tuples"},
        {"get_field", py_get_field, METH_VARARGS, "Return information about "
            "the map, as a dict, with items w and h for the width and height, "
            "and (x, y) mapping to \"rock\", \"brick\", \"empty\", "
            "\"explosion\" or a number for a bomb giving the time before it "
            "explodes (in seconds)"},

        {NULL, NULL, 0, NULL}
    };

    Py_Initialize();
    PyObject *module = Py_InitModule("bomberlua", methods);
    PyImport_AddModule("bomberlua");
    PyObject *bomber = PyCObject_FromVoidPtr((void *)this, NULL);
    PyModule_AddObject(module, "__bomber__", bomber);

    lock();

    // Initialization of the script: it has 5s to think and needs to all ready()
    // when done
    m_Thread = SDL_CreateThread(PyBomber::run, this);

    // The SDL_CondSignal cannot happen before SDL_CondWaitTimeout because the
    // mutex is locked

    int cond_ret = SDL_CondWaitTimeout(m_CondScriptActed, m_Mutex, 5000);
    if(cond_ret != 0)
    {
        std::cerr << "Initialization failed (script killed after 5 seconds)\n";

        // If the script takes too much time, we kill it
        SDL_KillThread(m_Thread);
        m_Thread = NULL;
        m_bAlive = false;
    }

    unlock();
}

PyBomber::~PyBomber()
{
    if(m_Thread)
        SDL_KillThread(m_Thread);
    if(m_CondScriptActed)
        SDL_DestroyCond(m_CondScriptActed);
    if(m_CondScriptResume)
        SDL_DestroyCond(m_CondScriptResume);
    if(m_Mutex)
        SDL_DestroyMutex(m_Mutex);
    Py_Finalize();
}

void PyBomber::update()
{
    // Send a signal to make the thread resume
    SDL_CondSignal(m_CondScriptResume);
}

int PyBomber::run(void *data)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    // Start the python script
    FILE *file = fopen(bomber->m_Filename.c_str(), "r");
    if(PyRun_SimpleFile(file, bomber->m_Filename.c_str()) != 0)
        std::cerr << "\"" << bomber->m_sFilename << "\" exited with an error"
            << std::endl;
    else
        std::cerr << "\"" << bomber->m_sFilename << "\" exited without error"
            << std::endl;

    bomber->lock();

    bomber->m_bAlive = false;

    bomber->unlock();

    return 0;
}

void PyBomber::kill()
{
    if(m_Thread)
    {
        SDL_KillThread(m_Thread);
        m_Thread = NULL;
    }
    m_bAlive = false;
}

void PyBomber::lock()
{
    SDL_LockMutex(m_Mutex);
}

void PyBomber::unlock()
{
    SDL_UnlockMutex(m_Mutex);
}
#endif
