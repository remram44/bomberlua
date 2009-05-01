/* v0.6 */

#ifndef CSOCKET_H
#define CSOCKET_H

#include <cstdio>
#include <cstring>           /* Pour memset() */
#include <exception>
#include <set>
#include <sstream>
#include <exception>

#ifdef __WIN32__
    #include <winsock2.h>

    typedef int socklen_t;
#else
    #include <sys/types.h>   /* Types prédéfinis "c" */
    #include <sys/socket.h>  /* Généralités sockets */
    #include <sys/param.h>   /* Paramètres et limites système */
    #include <netinet/in.h>  /* Spécifications socket internet */
    #include <arpa/inet.h>   /* Adresses format "arpanet" */
    #include <signal.h>      /* Signaux de communication */
    #include <netdb.h>       /* Gestion network database */
    #include <errno.h>       /* Erreurs système */
    #include <unistd.h>      /* Je sais pas mais Lockless l'utilise */

    #define closesocket close
#endif


/*==============================================================================
 * Les erreurs.
 *
 * Classe SocketError et classes dérivées.
 */

 /**
 * \brief Une erreur sur les sockets
 *
 * Classe de base pour héritage.
 */
class SocketError : public std::exception {
public:
    virtual const char *what() = 0;
};

/**
 * \brief Erreur fatale.
 *
 * Erreur qui ne devrait pas arriver et qui n'est donc pas causée par
 * l'utilisation que l'application fait de cette lib. Si une telle exception
 * est lancée, la lib est inutilisable. Cela peut venir d'une erreur du système
 * ou d'une limitation du système (par exemple d'un pare-feu, quoique peu
 * probable).
 */
class SocketFatalError : public SocketError {
public:
    const char *what();
};

/**
 * \brief Hôte introuvable.
 *
 * Si l'hôte auquel on cherche à se connecter n'a pas pu être trouvé via une
 * résolution de nom.
 */
class SocketUnknownHost : public SocketError {
public:
    const char *what();
};

/**
 * \brief Connexion refusée.
 *
 * Si l'ordinateur auquel on se connecte existe sur le réseau mais qu'il n'est
 * pas possible de se connecter sur le port spécifié.
 */
class SocketConnectionRefused : public SocketError {
public:
    const char *what();
};

/**
 * \brief Connexion fermée.
 *
 * Si la connexion a été fermée et n'est plus utilisable.
 */
class SocketConnectionClosed : public SocketError {
public:
    const char *what();
};

/**
 * \brief Impossible d'utiliser ce port.
 *
 * Si on veut attendre les connexions sur un port déjà utilisé ou que l'on a pas
 * le droit d'utiliser.
 */
class SocketCantUsePort : public SocketError {
public:
    const char *what();
};


/*============================================================================*/

/**
 * \defgroup net Moteur réseau
 */
/** @{ */
/**
 * \brief Une socket.
 *
 * Cette classe contient une socket. On compte le nombre de Sockets possédant
 * un pointeur sur chaque instance de SafeSocket afin de libérer les sockets
 * inutilisées automatiquement.
 */
class SafeSocket {

public:
    /**
     * \brief Etat de la socket
     */
    enum EType {
        SOC_CLOSED, /**< Socket fermée */
        SOC_SERVER, /**< Socket serveur */
        SOC_CLIENT  /**< Socket client */
    };

private:
    EType m_eType;  /**< Type de socket. */
    int m_iSocket;        /**< La socket. */
    int m_iRefs;          /**< Nombre de références sur cette socket. */

public:
    /**
     * \brief Constructeur.
     */
    SafeSocket(EType eType, int sock = -1);

public:
    /**
     * \brief Ajoute ou enlève une référence à cette socket.
     */
    void Ref(int chg);

    /**
     * \brief Retourne la socket.
     */
    inline int GetSocket()
    {
        return m_iSocket;
    }

    /**
     * \brief Retourne le type de socket.
     */
    inline EType GetType()
    {
        return m_eType;
    }

    /**
     * \brief Fermer la socket.
     *
     * Elle n'est pas détruite tant qu'il existe des références dessus.
     */
    void Close();

private:
    /**
     * \brief Destructeur.
     */
    ~SafeSocket();

};


/*============================================================================*/

/**
 * \brief Une socket.
 *
 * Classe de base.
 */
class Socket {

    friend class ClientSocket;
    friend class ServerSocket;
    friend class SocketSet;

    //! Tests
    friend bool operator==(const Socket& s1, const Socket& s2);
    //! Tests
    friend bool operator<(const Socket& s1, const Socket& s2);

protected:
    SafeSocket *m_pSocket; /**< La socket. */

public:
    /**
     * \brief Constructeur de base.
     */
    Socket();

    /**
     * \brief Constructeur de copie.
     */
    Socket(const Socket& sock);

    /**
     * \brief Destructeur.
     */
    virtual ~Socket();

    /**
     * \brief Assignation.
     */
    virtual const Socket& operator=(const Socket& sock);

    /**
     * \brief Attend un changement dans la socket.
     *
     * \param timeout temps d'attente en millisecondes. 0 retourne immédiatement
     * qu'il y ait eu un changement ou non, et -1 signifie d'attendre
     * indéfiniement.
     * \return true si la socket a été modifiée (si des données sont lisibles,
     * dans le cas d'une ClientSocket, ou si quelqu'un tente de se connecter,
     * pour une ServerSocket) pendant le temps défini, ou false si l'attente à
     * atteint la durée spécifiée.
     */
    bool Wait(int timeout = -1);

    /**
     * \brief Fermeture.
     *
     * Ferme la socket si elle est ouverte.
     */
    void Close();

    /**
     * \brief Indique que la Socket est invalide.
     */
    inline bool Invalid()
    {
        return m_pSocket == NULL;
    }

    /**
     * \brief Accesseur de la socket.
     *
     * On ne sait jamais...
     */
    inline int GetSocket()
    {
        if( (m_pSocket != NULL)
         && (m_pSocket->GetType() != SafeSocket::SOC_CLOSED) )
            return m_pSocket->GetSocket();
        else
            return -1;
    }

public:
    /**
     * \brief Initialise la lib.
     */
    static void Init();

};

/**
 * \brief Test deux socket pour indiquer si elles sont égales.
 */
bool operator==(const Socket& s1, const Socket& s2);

/**
 * \brief Relation d'ordre entre deux sockets. Utile pour les conteneurs triés comme
 * std::set (utilisé dans SocketSet) ou std::map.
 */
bool operator<(const Socket& s1, const Socket& s2);


/*============================================================================*/

/**
 * \brief Une socket client.
 *
 * Permet de communiquer en réseau.
 */
class ClientSocket : public Socket {

public:
    /**
     * \brief Constructeur par défaut.
     */
    ClientSocket();

    /**
     * \brief Constructeur à partir d'une socket.
     */
    ClientSocket(int sock);

    /**
     * \brief Constructeur de copie.
     *
     * Si un mauvais type de socket est passé, la ClientSocket est invalide.
     */
    ClientSocket(const Socket& sock);

    /**
     * \brief Assignation.
     */
    const Socket& operator=(const Socket& sock);

    /**
     * \brief Destructeur.
     */
    ~ClientSocket();

    /**
     * \brief Connexion à un serveur.
     *
     * \param hote Adresse de l'hôte, par exemple "www.debian.com".
     * \param port Numéro de port où se connecter.
     */
    void Connect(const char *hote, int port);

    /**
     * \brief nvoie des données.
     * \param donnes Données brutes à envoyer
     * \param size Tailles des données
     */
    void Send(const char *donnees, size_t size);

    /**
     * \brief Reçoit des données.
     *
     * \param bWait Indique si on doit attendre que des données arrive. Si false
     * est passé, la fonction peut retourner 0.
     * \return Le nombre d'octets reçus.
     */
    int Recv(char *donnees, int size_max, bool bWait = true);

    /**
     * \brief Envoie des données sous forme textuelle (utiliser Send pour des données
     * binaires).
     */
    template<class T>
    ClientSocket& operator<<(const T& donnee)
    {
        std::ostringstream os;
        os<<donnee;
        const char *s = os.str().c_str();
        Send(s, strlen(s));
        return *this;
    }

    /**
     * \brief Indique s'il y a des choses à recevoir sur cette socket.
     */
    bool Readable();

    /**
     * \brief Indique si la socket est connectée.
     */
    inline bool Connected()
    {
        return (m_pSocket != NULL)
            && (m_pSocket->GetType() == SafeSocket::SOC_CLIENT);
    }

};


/*============================================================================*/

/**
 * \brief Une socket serveur.
 *
 * Permet de se mettre en écoute sur un port et d'accepter des connexions, en
 * récupérant ainsi des ClientSocket sur les clients.
 */
class ServerSocket : public Socket {

public:
    /**
     * \brief Constructeur.
     */
    ServerSocket();

    /**
     * \brief Constructeur de copie.
     *
     * Si un mauvais type de socket est passé, la ClientSocket est invalide.
     */
    ServerSocket(const Socket& sock);

    /**
     * \brief Assignation.
     */
    const Socket& operator=(const Socket& sock);

    /**
     * \brief Destructeur.
     */
    ~ServerSocket();

    /**
     * \brief Mise en écoute sur un port.
     *
     * @param port Numéro de port sur lequel attendre des connexions.
     */
    void Listen(int port);

    /**
     * \brief Accepte une connexion.
     *
     * \param timeout Temps (en millisecondes) pendant lequel attendre une
     * connexion. Si timeout vaut 0, la méthode se termine tout de suite, et si
     * timeout est négatif on attend indéfiniement.
     * \return Une socket connectée au client, ou une socket invalide si aucun
     * client ne s'est connecté après le temps spécifié.
     */
    ClientSocket Accept(int timeout = 0);

    /**
     * \brief Indique si la socket est en écoute.
     */
    inline bool Listening()
    {
        return (m_pSocket != NULL)
            && (m_pSocket->GetType() == SafeSocket::SOC_SERVER);
    }

};


/*============================================================================*/

/**
 * \brief Un groupe de sockets.
 *
 * En plaçant plusieurs sockets dans un groupe de socket, on peut mettre le
 * processus en repos jusqu'à ce que quelque chose arrive sur l'une d'entre
 * elles.
 */
class SocketSet {

private:
    std::set<Socket> m_Sockets;

public:
    /**
     * \brief Constructeur.
     */
    SocketSet();

    /**
     * \brief Destructeur.
     */
    ~SocketSet();

    /**
     * \brief Indique si une socket se trouve dans le groupe.
     *
     * Surtout utilisé en interne mais peut-être que ça peut servir ?
     */
    bool IsSet(Socket sock);

    /**
     * \brief Ajouter une socket au groupe.
     */
    void AddSocket(Socket sock);

    /**
     * \brief Retirer une socket du groupe.
     *
     * \return true si la socket était dans le groupe, false sinon.
     */
    bool RemoveSocket(Socket sock);

    /**
     * \brief Attendre un changement dans le groupe de socket.
     *
     * \param timeout 0 retourne immédiatement qu'il y ait eu un changement ou
     * non, et -1 signifie d'attendre indéfiniement.
     */
    Socket Wait(int timeout = -1);

};
/** @} */
#endif
