/* v0.6 */

#include "Socket.h"
#include <iostream>

SafeSocket::SafeSocket(SafeSocket::EType eType, int sock)
  : m_eType(eType), m_iRefs(1)
{
    if(sock == -1)
    {
        m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_iSocket == -1)
            throw SocketFatalError();
    }
    else
        m_iSocket = sock;
}

SafeSocket::~SafeSocket()
{
    Close();
}

void SafeSocket::Ref(int chg)
{
    m_iRefs += chg;

    if(m_iRefs == 0)
        delete this;
}

void SafeSocket::Close()
{
    if(m_iSocket != -1)
    {
        closesocket(m_iSocket);
        m_iSocket = -1;
        m_eType = SafeSocket::SOC_CLOSED;
    }
}

/*============================================================================*/

const char *SocketFatalError::what()
{
    return "Fatal error";
}

const char *SocketUnknownHost::what()
{
    return "Unknown host";
}

const char *SocketConnectionRefused::what()
{
    return "Connection refused";
}

const char *SocketConnectionClosed::what()
{
    return "Connection closed";
}

const char *SocketCantUsePort::what()
{
    return "Can't use port";
}

/*============================================================================*/

void Socket::Init()
{
    static bool bInit = false;
    if(!bInit)
    {
#ifdef __WIN32__
        // Initialisation de winsock (sous windows)
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(1, 1), &wsa) != 0)
            throw SocketFatalError();
#endif
    }
}

bool operator==(const Socket& s1, const Socket& s2)
{
    return s1.m_pSocket == s2.m_pSocket;
}

bool operator<(const Socket& s1, const Socket& s2)
{
    return s1.m_pSocket < s2.m_pSocket;
}

Socket::Socket()
  : m_pSocket(NULL)
{
}

Socket::Socket(const Socket& sock)
  : m_pSocket(NULL)
{
    *this = sock;
}

const Socket& Socket::operator=(const Socket& sock)
{
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);

    m_pSocket = sock.m_pSocket;

    if(m_pSocket != NULL)
    {
        m_pSocket->Ref(+1);
    }

    return *this;
}

Socket::~Socket()
{
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);
}

bool Socket::Wait(int timeout)
{
    if( (m_pSocket == NULL)
     || (m_pSocket->GetType() == SafeSocket::SOC_CLOSED) )
        throw SocketConnectionClosed();

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_pSocket->GetSocket(), &fds);

    if(timeout < 0)
        select(m_pSocket->GetSocket() + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select(m_pSocket->GetSocket() + 1, &fds, NULL, NULL, &tv);
    }

    return FD_ISSET(m_pSocket->GetSocket(), &fds);
}

void Socket::Close()
{
    if(m_pSocket != NULL)
    {
        m_pSocket->Close();
        m_pSocket->Ref(-1);
        m_pSocket = NULL;
    }
}

/*============================================================================*/

ClientSocket::ClientSocket()
{
}

ClientSocket::ClientSocket(int sock)
{
    m_pSocket = new SafeSocket(SafeSocket::SOC_CLIENT, sock);
}

ClientSocket::ClientSocket(const Socket& sock)
{
    *this = sock;
}

const Socket& ClientSocket::operator=(const Socket& sock)
{
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);

    m_pSocket = sock.m_pSocket;

    if(m_pSocket != NULL)
    {
        if(m_pSocket->GetType() != SafeSocket::SOC_CLIENT)
            m_pSocket = NULL;
        else
            m_pSocket->Ref(+1);
    }

    return *this;
}

ClientSocket::~ClientSocket()
{
}

void ClientSocket::Connect(const char *hote, int port)
{
    // Si cette socket était connectée, la connexion a une référence de moins
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);

    // On créé la connexion
    m_pSocket = new SafeSocket(SafeSocket::SOC_CLIENT);

    // Résolution de l'adresse
    struct sockaddr_in adresse;
    struct hostent* h = gethostbyname(hote);
    if(h == NULL)
    {
        m_pSocket->Close();
        m_pSocket->Ref(-1);
        // Socket libérée automatiquement
        m_pSocket = NULL;

        throw SocketUnknownHost();
    }

    adresse.sin_family = AF_INET;
    adresse.sin_addr = *((struct in_addr *)h->h_addr);
    adresse.sin_port = htons(port);

    memset(&(adresse.sin_zero), 0, 8);

    // Ouverture de la connexion
    if(connect(m_pSocket->GetSocket(),
               (struct sockaddr*)&adresse, sizeof(adresse)) == -1)
    {
        m_pSocket->Close();
        m_pSocket->Ref(-1);
        // La SafeSocket est détruite automatiquement
        m_pSocket = NULL;

        throw SocketConnectionRefused();
    }
}

void ClientSocket::Send(const char *donnees, size_t size)
{
    if( (m_pSocket == NULL)
     || (m_pSocket->GetType() != SafeSocket::SOC_CLIENT) )
        throw SocketConnectionClosed();

    if(send(m_pSocket->GetSocket(), donnees, size, 0) != (int)size)
    {
        m_pSocket->Close();
        m_pSocket->Ref(-1);
        m_pSocket = NULL;
        throw SocketConnectionClosed();
    }
}

int ClientSocket::Recv(char *donnees, int size_max, bool bWait)
{
    if( (m_pSocket == NULL)
     || (m_pSocket->GetType() != SafeSocket::SOC_CLIENT) )
        throw SocketConnectionClosed();

    if(bWait || Readable())
    {
        int ln = recv(m_pSocket->GetSocket(), donnees, size_max, 0);
        if(ln <= 0)
        {
            m_pSocket->Close();
            m_pSocket->Ref(-1);
            m_pSocket = NULL;
            throw SocketConnectionClosed();
        }
        else
            return ln;
    }
    else
        return 0;
}

bool ClientSocket::Readable()
{
    return Wait(0);
}

/*============================================================================*/
ServerSocket::ServerSocket()
{
}

ServerSocket::ServerSocket(const Socket& sock)
{
    *this = sock;
}

const Socket& ServerSocket::operator=(const Socket& sock)
{
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);

    m_pSocket = sock.m_pSocket;

    if(m_pSocket != NULL)
    {
        if(m_pSocket->GetType() != SafeSocket::SOC_SERVER)
            m_pSocket = NULL;
        else
            m_pSocket->Ref(+1);
    }

    return *this;
}

ServerSocket::~ServerSocket()
{
}

void ServerSocket::Listen(int port)
{
    // Si cette socket était connectée, la connexion a une référence de moins
    if(m_pSocket != NULL)
        m_pSocket->Ref(-1);

    // On créé la connexion
    m_pSocket = new SafeSocket(SafeSocket::SOC_SERVER);

    struct sockaddr_in sin;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if(
    (bind(m_pSocket->GetSocket(), (struct sockaddr*)&sin, sizeof(sin) ) == -1)
     || (listen(m_pSocket->GetSocket(), 5) == -1) )
    {
        m_pSocket->Close();
        m_pSocket->Ref(-1);
        // La SafeSocket est détruite automatiquement
        m_pSocket = NULL;
        throw SocketCantUsePort();
    }
}

ClientSocket ServerSocket::Accept(int timeout)
{
    if( (m_pSocket == NULL)
     || (m_pSocket->GetType() != SafeSocket::SOC_SERVER) )
        throw SocketConnectionClosed();

    if(Wait(timeout))
    {
        struct sockaddr_in clientsin;
        socklen_t taille = sizeof(clientsin);
        int sock = accept(m_pSocket->GetSocket(),
            (struct sockaddr*)&clientsin, &taille);

        if(sock != -1)
            return ClientSocket(sock);
        else
        {
            m_pSocket->Close();
            m_pSocket->Ref(-1);
            m_pSocket = NULL;
            return ClientSocket();
        }
    }

    return ClientSocket();
}

SocketSet::SocketSet()
{
}

SocketSet::~SocketSet()
{
    m_Sockets.clear();
}

bool SocketSet::IsSet(Socket sock)
{
    std::set<Socket>::iterator it;
    it = m_Sockets.find(sock);
    return it != m_Sockets.end();
}

void SocketSet::AddSocket(Socket sock)
{
    if(!IsSet(sock))
        m_Sockets.insert(sock);
}

bool SocketSet::RemoveSocket(Socket sock)
{
    std::set<Socket>::iterator it;
    it = m_Sockets.find(sock);
    if(it != m_Sockets.end())
    {
        m_Sockets.erase(it);
        return true;
    }
    else
        return false;
}

Socket SocketSet::Wait(int timeout)
{
    fd_set fds;
    FD_ZERO(&fds);

    int greatest = -1;

    std::set<Socket>::iterator it = m_Sockets.begin();
    while(it != m_Sockets.end())
    {
        if(it->m_pSocket->GetSocket() > greatest)
            greatest = it->m_pSocket->GetSocket();
        FD_SET(it->m_pSocket->GetSocket(), &fds);
        it++;
    }

    if(greatest == -1)
        return Socket();

    if(timeout == -1)
        select(greatest + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select(greatest + 1, &fds, NULL, NULL, &tv);
    }

    it = m_Sockets.begin();
    while(it != m_Sockets.end())
    {
        if(FD_ISSET(it->m_pSocket->GetSocket(), &fds))
            return *it;
        else
            it++;
    }

    return Socket();
}
