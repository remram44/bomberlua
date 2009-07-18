#ifndef BOMBERLUA_H
#define BOMBERLUA_H

#include <list>
#include <vector>
#include <string>

/**
 * Compteur.
 *
 * Retourne le nombre de secondes écoulées depuis le lancement du programme.
 */
double getTicks();

/**
 * Un moteur.
 *
 * Le moteur est la partie qui fournit les données au programme. Cela peut être
 * fait en récupérant ces données depuis un serveur, ou en calculant nous-même
 * l'état du jeu via le moteur du jeu (GameEngine).
 * D'un autre côté, le module Output s'occupe de la sortie du programme ; il est
 * capable d'afficher les données avec SDL et/ou de les envoyer à des clients
 * par le réseau.
 */
class Engine {

protected:
    int m_iWidth, m_iHeight;

public:
    /**
     * Les types possibles pour les "cases" de la map.
     */
    enum ECell {
        CELL_ROCK,  /**< Mur indesctructible */
        CELL_BRICK, /**< Mur destructible */
        CELL_EMPTY  /**< Rien */
    };

    /**
     * Les actions possibles pour les personnages.
     */
    enum EAction {
        ACT_IDLE,
        ACT_MOV_LEFT,
        ACT_MOV_RIGHT,
        ACT_MOV_UP,
        ACT_MOV_DOWN,
        ACT_DROP_BOMB
    };

    /**
     * Un personnage.
     */
    class Bomber {

    public:
        /** Nom */
        std::string m_sFilename;

        /** Position */
        int m_iPosX, m_iPosY;

        /** Indique si le personnage est en vie.
         * Un personnage peut être mis hors-jeu soit s'il est tué, soit si son
         * IA bug (soit en provoquant une erreur Lua, soit en cas de "timeout",
         * c'est à dire si elle tarde trop à faire une action). */
        bool m_bAlive;

        /** Action en cours */
        EAction m_eAction;

        /** Date à laquelle l'action a commencé */
        double m_dBeginAction;

    public:
        /**
         * Constructeur.
         */
        Bomber(const char *filename, int posX, int posY);
    };

    /**
     * Une bombe.
     */
    class Bomb {

    public:
        /** Position */
        int m_iPosX, m_iPosY;

        /** Date à laquelle la bombe explosera */
        double m_dExplodeDate;

        /** Puissance (= nombre de cases qui seront touchées de chaque côté) */
        int m_iCells;

    public:
        /**
         * Constructeur.
         */
        Bomb(int posX, int posY, double explodeDate, int cells);

    };

public:
    /**
     * Mise à jour.
     *
     * Met à jour les données.
     */
    virtual bool update() = 0;

    /** Accesseur de la largeur de la map. */
    inline int getMapWidth()  const { return m_iWidth;  }
    /** Accesseur de la hauteur de la map. */
    inline int getMapHeight() const { return m_iHeight; }

    /**
     * Retourne la map.
     */
    virtual const std::vector<ECell>& getMap() const = 0;

    /**
     * Retourne les personnages.
     */
    virtual std::vector<const Bomber*> getBombers() const = 0;

    /**
     * Retourne les bombes.
     */
    virtual std::vector<const Bomb*> getBombs() const = 0;

    /**
     * Retourne la carte des explosions.
     */
    virtual const std::vector<double>& getExplosions() const = 0;

    virtual ~Engine();

    /**
     * Début de l'output
     *
     * Cette méthode peut être utile à l'engine pour "préparer" les objets
     * à être rendus.
     */
    virtual void beginOutput();

    /**
     * Fin de l'output.
     *
     * L'output est terminé et l'engine peut effectuer quelques actions.
     */
    virtual void endOutput();

};

/**
 * L'application.
 *
 * Lit les paramètres depuis la ligne de commande afin d'initialiser le module
 * de sortie (Output) et le moteur (classe abstraite Engine).
 */
class BomberLua {

private:
    /**
     * Indique si le jeu est en fonctionnement. S'il ne l'est plus,
     * l'application se termine.
     */
    bool m_bRunning;

    /**
     * Le moteur de jeu.
     *
     * Contient les données et dispose des méthodes permettant de les mettre à
     * jour.
     */
    Engine *m_pEngine;

private:
    /**
     * Constructeur.
     */
    BomberLua();

    /**
     * Destructeur.
     */
    ~BomberLua();

    /**
     * Retourne un pointeur sur l'instance (modèle Singleton).
     */
    static BomberLua *get()
    {
        static BomberLua app;
        return &app;
    }

    /**
     * Initialise le jeu en lisant la ligne de commande.
     */
    int init(int argc, char **argv);

    /**
     * Fonctionnement de l'application (boucle principale).
     */
    int run();

public:
    /**
     * Lance le jeu (méthode appelée par main()).
     */
    static int launch(int argc, char **argv);

};

#endif
