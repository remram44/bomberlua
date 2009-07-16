#ifdef WITH_PYTHON
#include "PyBomber.h"

#include <iostream>
#include <sstream>

PyObject *PyBomber::py_log(PyObject *self, PyObject *args)
{
    const char *text;
    if (!PyArg_ParseTuple(args, "s", &text))
        return NULL;
    std::cerr << "Log : " << text << "\n";
    Py_INCREF(Py_None);
    return Py_None;
}
PyObject *PyBomber::py_ready(PyObject *self, PyObject *args)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    bomber->lock();

    // On indique que le script a agit
    SDL_CondSignal(bomber->m_CondScriptActed);

    // On attend un signal pour reprendre l'exécution
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

    // On indique que le script a agit
    SDL_CondSignal(bomber->m_CondScriptActed);

    // On attend un signal pour reprendre l'exécution
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

    // On indique que le script a agit
    SDL_CondSignal(bomber->m_CondScriptActed);

    // On attend un signal pour reprendre l'exécution
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
            //On crée les positions du bomber
            PyList_Append(list, 
                Py_BuildValue("(i,i)", it->m_iPosX, it->m_iPosY)); 
        }
        //On les ajoutes à la liste 
    }
    std::cerr << "\n";
    return list;
}

PyObject *PyBomber::py_get_field(PyObject *self, PyObject *args)
{
    // On récupère un pointeur sur le Bomber associé
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    GameEngine *engine = bomber->m_pEngine;

    //On crée le dictionnaire
    //qui sera sous la forme :
    //{"w":2, "h":2, (0,0):"empty" ...}
    PyObject *ret = PyDict_New();
    // On y place la taille
    int width = engine->getMapWidth();
    int height = engine->getMapHeight();
    PyDict_SetItemString(ret, "w", Py_BuildValue("i", width));
    PyDict_SetItemString(ret, "h", Py_BuildValue("i", height));


    // Puis on boucle
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
            // Mur indestructible
            case Engine::CELL_ROCK:
                type = Py_BuildValue("s", "rock");
                break;
            // Mur destructible
            case Engine::CELL_BRICK:
                type = Py_BuildValue("s", "brick");
                break;
            // Pas de mur ; mais peut-être une bombe ou une explosion ?
            case Engine::CELL_EMPTY:
                // Une explosion
                if(explosions[y * width + x]
                 >= (SDL_GetTicks()/1000.0) )
                    type = Py_BuildValue("s", "explosion");
                else
                {
                    // Une bombe
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
    // Crée la condition indiquant que le script a fini son exécution
    m_CondScriptActed = SDL_CreateCond();

    // Crée la condition indiquant que le moteur attend une nouvelle action, et
    // que le script doit reprendre son exécution
    m_CondScriptResume = SDL_CreateCond();

    // Crée le mutex
    m_Mutex = SDL_CreateMutex();

    //Charge python
    static PyMethodDef methods[] = {
        {"log", py_log, METH_VARARGS, "Permet d'enregistrer des informations"},
        {"ready", py_ready, METH_VARARGS, 
            "Indique que le script est initialisé"},
        {"move", py_move, METH_VARARGS, "Se déplace. Les paramètres"
            "possibles sont \"left\", \"right\", \"up\", \"down\""},
        {"bomb", py_bomb, METH_VARARGS, "Pose une bombe"},
        {"get_self", py_get_self, METH_VARARGS, 
            "Retourne la position du joueur sous forme de tuple"},
        {"get_bombers", py_get_bombers, METH_VARARGS, "Retourne les positions"
            " des autres joueurs sous forme de liste de tuple"},
        {"get_field", py_get_field, METH_VARARGS, "Retourne les informations"
            " sur la map, sous forme de dico, avec le champ w et h pour la "
            "hauteur et largeur, et (x,y) avec le type de chaque case "
            "\"rock\", \"brick\", \"empty\", \"explosion\" ou un nombre "
            "représentant le nombre de seconde avant l'explosion)"},

        {NULL, NULL, 0, NULL}
    };

    Py_Initialize();
    PyObject *module = Py_InitModule("bomberlua", methods);
    PyImport_AddModule("bomberlua");
    PyObject *bomber = PyCObject_FromVoidPtr((void *)this, NULL);
    PyModule_AddObject(module, "__bomber__", bomber);

    // On vire les fonctions et tables dangereuses, dont les scripts n'ont pas
    // besoin
    /*
    const char *const unset[13] = {
        "os", // fonctions dangereuses et inutiles au script
        "io", // permet d'accéder au système de fichiers
        "debug", // permet d'accéder à des informations sur le programme
        "package",
        "module",
        "getfenv", // permet d'accéder à l'environnement de l'appelant
        "setfenv",
        "collectgarbage", // lua collectera les détritus automatiquement
        "gcinfo",
        "newproxy",
        "dofile", "require", "loadfile" // permet d'étendre Lua en chargeant
        // des modules
        };

    int i;
    for(i = 0; i < 14; i++)
    {
        char cmd[256] = "";
        strcat(cmd, unset[i]);
        strcat(cmd, " = None");

        PyRun_SimpleString(cmd);
    }
    */


    lock();

    // Initialisation du script : il a 5s pour s'initialiser et doit appeler
    // ready() lorsqu'il est prêt
    m_Thread = SDL_CreateThread(PyBomber::run, this);

    // Le SDL_CondSignal ne peut pas avoir lieu avant l'appel à
    // SDL_CondWaitTimeout car le Mutex est locké

    int cond_ret = SDL_CondWaitTimeout(m_CondScriptActed, m_Mutex, 5000);
    if(cond_ret != 0)
    {
        std::cerr << "Initialisation ratée (script killé après 5 secondes)\n";

        // Si le script prend trop de temps, on le kill
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
    // On envoie un signal pour indiquer au thread de reprendre
    SDL_CondSignal(m_CondScriptResume);
}

int PyBomber::run(void *data)
{
    PyObject *module = PyImport_ImportModule("bomberlua");
    PyObject *b = PyObject_GetAttrString(module, "__bomber__");
    PyBomber *bomber = static_cast<PyBomber*>(((void *)PyCObject_AsVoidPtr(b)));

    // Lance le script python
    FILE *file = fopen(bomber->m_Filename.c_str(), "r");
    if(PyRun_SimpleFile(file, bomber->m_Filename.c_str()) != 0)
        std::cerr << "\"" << bomber->m_sFilename << "\" s'est terminé avec une "
            "erreur" << std::endl;
    else
        std::cerr << "\"" << bomber->m_sFilename << "\" s'est terminé sans "
            "erreur" << std::endl;

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
