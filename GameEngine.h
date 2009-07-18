#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "BomberLua.h"
#include <list>
#include <vector>
#include <string>
#include <SDL/SDL_thread.h>

#ifdef WITH_LUA
class LuaBomber;
#endif
#ifdef WITH_PYTHON
class PyBomber;
#endif

/**
 * \defgroup gameengine Moteur du jeu
 */
/** @{ */
/**
 * Le moteur du jeu.
 *
 * Ce moteur est le moteur "principal", qui génère les données en faisant
 * combattre les IAs.
 */
class GameEngine : public Engine {

public:
    /**
     * Un joueur controllé par un script.
     */
    class IABomber : public Engine::Bomber {

    protected:
        GameEngine *m_pEngine;
        unsigned int m_iBombRange;

    protected:
        void wait();
        bool move(const std::string &param);
        void bomb();

    public:
        /**
         * Constructeur.
         *
         * Charge le script et l'initialise.
         * @param filename Nom du fichier à utiliser.
         */
        IABomber(GameEngine *engine, int startx, int starty,
            const char *filename);

        /**
         * Destructeur.
         */
        virtual ~IABomber() {}

        /**
         * Reprend l'exécution du script dans un thread.
         */
        virtual void update() = 0;

        /**
         * Tue ce personnage.
         *
         * Il n'est alors plus vivant, et le thread est tué s'il était actif.
         */
        virtual void kill() = 0;

        /**
         * Verrouille le personnage.
         *
         * Tout accès aux membres du personnage nécessite son verrouillage, car
         * ils sont succeptibles d'être utilisés également depuis les threads.
         */
        virtual void lock() = 0;

        /**
         * Déverrouille le personnage.
         */
        virtual void unlock() = 0;

    };

private:
    std::vector<Engine::ECell> m_Map;
    std::vector<IABomber*> m_Bombers;
    std::list<Engine::Bomb*> m_Bombs;
    std::vector<double> m_Explosions;

public:
    /**
     * Accesseur de la map.
     *
     * Permet d'obtenir les données sur les cases de la map.
     */
    const std::vector<Engine::ECell>& getMap() const;

    /**
     * Accesseur des personnages.
     */
    std::vector<const Engine::Bomber*> getBombers() const;

    /**
     * Accesseur des bombes.
     */
    std::vector<const Engine::Bomb*> getBombs() const;

    /**
     * Accesseur des explosions.
     */
    const std::vector<double>& getExplosions() const;

public:
    /**
     * Constructeur.
     *
     * @param progs Liste des programmes à charger.
     */
    GameEngine(std::list<std::string> progs);

    /**
     * Destructeur.
     */
    ~GameEngine();

    /**
     * Se met à jour en exécutant une frame.
     *
     * @return false si la partie est terminée, true sinon.
     */
    bool update();

    /**
     * Début de l'output.
     *
     * Fonction surchargée afin de locker les personnages.
     */
    void beginOutput();

    /**
     * Fin de l'output.
     *
     * Fonction surchargée afin d'unlocker les personnages.
     */
    void endOutput();

};
/** @} */

#ifdef WITH_LUA
#include "LuaBomber.h"
#endif
#ifdef WITH_PYTHON
#include "PyBomber.h"
#endif

#endif
