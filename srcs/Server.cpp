#include "../incs/Server.hpp"
#include "../incs/Reply.hpp"

static volatile sig_atomic_t g_running = 1;
static void	signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
	std::cout << "Closing server..." << std::endl;
}

/* <--- constructors ---> */
Server::Server() : _port(6667), _ip("127.0.0.1"), _serverFd(-1), _opt(1), _password("42"), _running(false), _opPwd("42")
{
    _start = std::clock();
    _bot = Client(Fd(-1), "BOT");
    _bot.setUserName("BOT");
    _bot.setPwdAccess(true);
    _bot.setIp("B.O.T.!");
    _bot.setPort(4242);
    _bot.setBot(true);
}

Server::~Server()
{
    std::cout << "Cleaning _clients container..." << std::endl;
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second)
            delete it->second;
    }
    _clients.clear();
    std::cout << "Cleaning _channels container..." << std::endl;
    for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (*it)
            delete *it;
    }
    _channels.clear();
}

Server::Server( const Server& other ) : _port(other._port), _ip(other._ip), _serverFd(other._serverFd), _opt(other._opt), _password(other._password), _address(other._address), _start(other._start) ,_running(other._running), _opPwd(other._opPwd)
{
    for (std::map<int, Client*>::const_iterator it = other._clients.begin(); it != other._clients.end(); ++it)
    {
        if (it->second)
        {
            Client* copy = new Client(*(it->second));
            _clients.insert(std::make_pair(copy->getFd(), copy));
        }
    }
}

Server& Server::operator=( const Server& other )
{
    if (this != &other)
    {
        for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second)
                delete it->second;
        }
        _clients.clear();
        this->_port = other._port;
        this->_ip = other._ip;
        this->_password = other._password;
        this->_serverFd.operator=(other._serverFd);
        this->_opt = other._opt;
        this->_address = other._address;
        for (std::map<int, Client*>::const_iterator it = other._clients.begin(); it != other._clients.end(); ++it)
        {
            if (it->second)
            {
                Client* copy = new Client(*(it->second));
                _clients.insert(std::make_pair(copy->getFd(), copy));
            }
        }
        this->_start = other._start;
        this->_running = other._running;
    }
    return (*this);
}

/* <--- init && start ---> */
void    Server::init( const std::string& argOne, const std::string& argTwo )
{
    if (argOne.length() > 5)
        throw(BadPort());
    for (size_t i = 0; i < argTwo.length(); i++)
        if (!std::isalnum(static_cast<char>(argTwo[i])))
            throw(BadPass());
    int  port = atoi(argOne.c_str());
    if (port < 1024 || port > 49151)
        throw (BadPort());
    this->setPassword(argTwo);
    this->setPort(port);
}

void    Server::initSocket( void )
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        throw (SocketCreationError());
    try
    {
        setFd(Fd(fd));
    }
    catch(...)
    {
        throw;
    }
    if (setsockopt(getFd(), SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt)) == -1)
        throw (SetsockoptError());
    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_port = htons(getPort());
    _address.sin_addr.s_addr = INADDR_ANY;
}

void    Server::bindSocket( void )
{
    if (bind(getFd(), (struct sockaddr *)&_address, sizeof(_address)) == -1)
        throw (BindError());
}

void    Server::listenSocket( void )
{
    if (listen(getFd(), 128) == -1)
        throw (ListenError());
    if (fcntl(getFd(), F_SETFL, O_NONBLOCK) == -1)
        throw (ListenError());
}

void    Server::pollLoop( void )
{
    std::map<int, std::string>  clientBuffers;
    std::vector<pollfd> fds;
    pollfd serverFd = { getFd(), POLLIN, 0 };
    fds.push_back(serverFd);
    while (g_running)
    {
        for (size_t idx = 0; idx < fds.size(); ++idx)
        {
            if (fds[idx].fd == getFd())
                fds[idx].events = POLLIN;
            else
            {
                Client* c = getClientByFd(fds[idx].fd);
                if (c && !c->getOutBuf().empty())
                    fds[idx].events = POLLIN | POLLOUT;
                else
                    fds[idx].events = POLLIN;
            }
        }
        int ret = poll(fds.data(), fds.size(), 1000);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            throw (pollError());
        }
        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents == 0)
            {
                checkClientsLastAction();
                continue ;
            }
            if (fds[i].revents & POLLOUT)
            {
                Client* writable = getClientByFd(fds[i].fd);
                if (writable && !writable->getOutBuf().empty())
                {
                    const std::string& out = writable->getOutBuf();
                    ssize_t s = send(writable->getFd(), out.c_str(), out.size(), MSG_NOSIGNAL);
                    if (s > 0)
                    {
                        writable->popOutBuf(static_cast<size_t>(s));
                    }
                    else if (s == -1)
                    {
						;//skip
                    }
                    else
                    {
                        Client* tmp = getClientByFd(fds[i].fd);
                        if (tmp)
                            removeClient(tmp);
                        clientBuffers.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue ;
                    }
                }
			}
            if (fds[i].fd == getFd())
            {
                if (fds[i].revents & POLLIN)
                {
                    sockaddr_storage    tmpAddr;
                    socklen_t           addrLen = sizeof(tmpAddr);
                    int rawFd = accept(getFd(), (sockaddr *)&tmpAddr, &addrLen);
                    if (rawFd >= 0)
                    {
                        Client* newClient = NULL;
                        try
                        {
                            newClient = new Client(Fd(rawFd));
                            rawFd = -1;
                        }
                        catch(...)
                        {
                            rawFd = -1;
                            continue;
                        }
                        struct sockaddr_in* s4 = (struct sockaddr_in*)&tmpAddr;
                        newClient->setIp(std::string(inet_ntoa(s4->sin_addr)));
                        newClient->setPort(static_cast<int>(ntohs(s4->sin_port)));
                        int dupFd = newClient->getFd();
                        if (dupFd < 0)
                        {
                            std::cerr << "Failed to create client fd from accepted socket" << std::endl;
                            delete newClient;
                            continue ;
                        }
                        fcntl(dupFd, F_SETFL, O_NONBLOCK);
                        pollfd  client = { dupFd, POLLIN, 0 };
                        fds.push_back(client);
                        addClient(newClient);
                        std::cout << "new connection from: " << newClient->getIp() << ":" << newClient->getPort() << std::endl;
                    }
                    else
                        continue ;
                }
                continue ;
            }
            if (fds[i].revents & (POLLIN))
            {
                char    tmp[4096];
                ssize_t bReads = recv(fds[i].fd, tmp, sizeof(tmp), 0);

                if (bReads > 0)
                {
                    clientBuffers[fds[i].fd].append(tmp, static_cast<size_t>(bReads));

                    std::string &buf = clientBuffers[fds[i].fd];
                    size_t  pos_n;
                    while ((pos_n = buf.find('\n')) != std::string::npos)
                    {
                        std::string line;
                        if (pos_n > 0 && buf[pos_n - 1] == '\r')
                            line = buf.substr(0, pos_n - 1);
                        else
                            line = buf.substr(0, pos_n);
                        buf.erase(0, pos_n + 1);
                        Client* user = getClientByFd(fds[i].fd);
                        std::cout << "Client " << (!user->getStatus() ? "***" : user->getNick()) << " send : " << line << std::endl;
                        Command::execute(this, user, line);
                    }
                }
                    else if (bReads == 0)
                    {
                        Client* logOutClient = getClientByFd(fds[i].fd);
                        if (logOutClient)
                            this->removeClient(logOutClient);
                        clientBuffers.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                        continue ;
                    }
                else
                {
                    continue;
                }
            }
            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                Client* logOutClient = getClientByFd(fds[i].fd);
                if (logOutClient)
                {
                    std::cout << "Client " << (logOutClient->getNick().empty() ? "***" : logOutClient->getNick()) << " disconnected." << std::endl;
                    this->removeClient(logOutClient);
                }
                clientBuffers.erase(fds[i].fd);
                fds.erase(fds.begin() + i);
                --i;
                continue ;
            }
        }
    }
}

void	Server::start( const std::string& argOne, const std::string& argTwo )
{
	init(argOne, argTwo);
    initSocket();
    bindSocket();
    listenSocket();
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    _running = true;
    pollLoop();
    _running = false;
}

void    Server::sendToClient( const std::string& raw, const Client* client, const std::string& code )
{
    if (!client)
        return ;
    std::string msg = reply_format(this, client, code, raw);
    const_cast<Client*>(client)->queueSend(msg);
}

void    Server::sendToChannel( const std::string& raw, const std::string& channel, Client* sender )
{
    Channel*    tmp = NULL;
    std::string msg;

    if (sender)
        msg = ":" + sender->getNick() + "!" + sender->getName() + "@" + sender->getIp()
                    + " PRIVMSG " + channel + " :" + raw + "\r\n";
    else
        msg = ":" + this->getIp() + " NOTICE " + channel + " :" + raw + "\r\n";
    for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if ((*it)->getName() == channel)
        {
            tmp = *it;
            break ;
        }
    }
    if (!tmp || tmp->getUserCount() == 0 || (sender && (std::find(tmp->getUsers().begin(), tmp->getUsers().end(), sender) == tmp->getUsers().end())))
        return (sendToClient("channel doesn't exist", sender, ""));
    for (std::vector<Client*>::const_iterator it = tmp->getUsers().begin(); it != tmp->getUsers().end(); ++it)
        {
            if (sender != NULL && *it == sender)
                continue;
            if (!msg.empty())
                (*it)->queueSend(msg);
            else
                sendToClient(raw, (*it), "");
        }
}

/* <--- setters ---> */
void    Server::setPort( int port )
{
    this->_port = port;
}

void    Server::setPassword( const std::string& password )
{
    this->_password = password;
}

void    Server::setFd( Fd fd )
{
    if (fd.getFd() < 0)
        throw (SocketCreationError());
    this->_serverFd = fd;
}

/* <--- getters ---> */

const std::string&  Server::getIp( void ) const
{
    return (this->_ip);
}

Channel*	Server::getChannelByName( const std::string& channel )
{
	Channel	*channelPtr = NULL;

	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if ((*it)->getName() == channel)
		{
			channelPtr = *it;
			return (channelPtr);
		}
	}
	return (channelPtr);
}

const std::vector<Channel*>& Server::getChannels( void ) const
{
    return (_channels);
}

int Server::getPort( void ) const
{
    return (this->_port);
}

const std::string& Server::getPassword( void ) const
{
    return (this->_password);
}

int Server::getFd( void ) const
{
    return (_serverFd.getFd());
}

void    Server::addClient( Client* client )
{
    if (!client)
        return ;
    _clients.insert(std::make_pair(client->getFd(), client));
}

Client*   Server::getClientByNick( const std::string& nick ) const
{
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second && it->second->getNick() == nick)
            return (it->second);
    }
    return (NULL);
}

Client* Server::getClientByFd( int fd ) const
{
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first == fd)
            return (it->second);
    }
    return (NULL);
}

const std::map<int, Client*>&	Server::getClients( void ) const
{
    return (_clients);
}

void    Server::removeClient( Client* client )
{
    if (!client)
        return ;
    const std::string   clientNick = client->getNick();

    if (client->getStatus())
    {
        for (std::vector<Channel*>::iterator chanIt = _channels.begin(); chanIt != _channels.end(); )
        {
            Channel* ch = *chanIt;
            if (!ch || ch->getName().empty())
            {
                ++chanIt;
                continue ;
            }
            if (std::find(ch->getUsers().begin(), ch->getUsers().end(), client) != ch->getUsers().end())
                ch->rmUser(client);
            if (ch->getOwner() == clientNick)
            {
                for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
                {
                    std::string	r = "Channel " + ch->getName() + " has been deleted";
                    sendToClient(r, *it, "");
                }
                chanIt = _channels.erase(chanIt);
                delete ch;
                continue ;
            }
            ++chanIt;
        }
    }
    std::map<int, Client*>::iterator    cliIt = _clients.find(client->getFd());
    if (cliIt != _clients.end())
    {
        std::cout << "connection closed from " << client->getIp() << "@" << client->getPort() << std::endl;
        delete cliIt->second;
        _clients.erase(cliIt);
    }
    if (_clients.size() == 0)
        return ;
}

void    Server::removeFd( int fd )
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return ;
    delete it->second;
    _clients.erase(it);
}

bool    Server::addUserToChannel( const std::string& channel, Client* client, const std::string& key )
{
    if (_channels.size() == 0)
        return (false);
    for(std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (channel == (*it)->getName())
        {
            Channel* ch = *it;
            if (ch->getMaxUsers() != 0 && ch->getMaxUsers() <= ch->getUserCount())
            {
                sendToClient("Channel is full", client, "471");
                return (false);
            }
            if (ch->hasMode('i'))
            {
                if (ch->getOwner() != client->getNick() && !ch->isOp(client) && !client->isOp() && !ch->isInvited(client->getNick()))
                {
                    sendToClient("Channel is invite only", client, "473");
                    return (false);
                }
            }
            if (ch->hasMode('k'))
            {
                if (key.empty() || ch->getPwd() != key)
                {
                    sendToClient("Bad channel key", client, "475");
                    return (false);
                }
            }
            if (ch->addUser(client))
            {
                if (ch->isInvited(client->getNick()))
                    ch->rmInvite(client->getNick());
                return (true);
            }
            return (false);
        }
    }
    return (false);
}

bool    Server::createChannel( const std::string& channel, Client* client )
{
	if (channel.find('#') != 0 || (channel.substr(1).find('#') != std::string::npos || !isStrAlnum(channel.substr(1))))
		return (false);
    if (client->getBanStatus())
        return (false);
	else if (_channels.size() > 0)
	{
		for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			if ((*it)->getName() == channel)
				return (false);
		}
	}
    Channel *newChannel = new Channel(channel);

    newChannel->setOwner(client);
    _channels.push_back(newChannel);
    if (addUserToChannel(channel, client))
    {
        newChannel->addOp(client);
        return (true);
    }
    std::vector<Channel*>::iterator remIt = std::find(_channels.begin(), _channels.end(), newChannel);
    if (remIt != _channels.end())
        _channels.erase(remIt);
    delete newChannel;
    return (false);
}

void    Server::deleteChannel( Channel *chan )
{
    std::vector<Channel*>::iterator  it;
    
    it = std::find(_channels.begin(), _channels.end(), chan);
    if (it == _channels.end())
        return ;
    sendToChannel("Your channel gonna be deleted because the owner left", chan->getName(), NULL);
    _channels.erase(it);
	delete chan;
}

void    Server::checkClientsLastAction( void )
{
    std::time_t now = std::time(NULL);
    if (_clients.size() == 0)
        return ;
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end();)
    {
        Client* c = it->second;
        ++it;
        if (c && c->getStatus())
        {
            if (c && !c->isPinged() && (now - c->getLastAction() >= PING_TIME))
            {
                std::ostringstream oss;
                oss << c->getLastAction();
                std::cout << "ping sent to " << c->getNick() << std::endl;
                sendToClient(std::string("PING :") + oss.str(), c, "");
                c->setPinged(true);
                c->setPingCode(c->getLastAction());
            }
            else if (c && c->isPinged() && (now - c->getLastAction() >= LOGOUT_TIME))
            {
                std::ostringstream  oss;
                oss << LOGOUT_TIME;
                sendToClient(std::string("ERROR : Closing Link: ") + c->getIp() + " (Ping timeout : " + oss.str() + " seconds)", c, "");
                removeClient(c);
                if (_clients.size() == 0)
                    return ;
            }
        }
        else
        {
            if (c && (now - c->getLastAction() >= REGISTRATION_TIME))
            {
                sendToClient(std::string("ERROR : Closing Link: ") + c->getIp() + " (Registration timed out)", c, "");
                removeClient(c);
                if (_clients.size() == 0)
                    return ;
            }
        }
    }
}

const std::string&	Server::getOpPwd( void ) const
{
	return (_opPwd);
}

const Client* Server::getBot( void ) const
{
    return (&_bot);
}

/* <--- exceptions ---> */

const char* Server::pollError::what() const throw()
{
    static std::string msg = std::string("Error in poll() : ") + std::strerror(errno);
    return (msg.c_str());
}

const char* Server::SocketCreationError::what() const throw()
{
    static std::string msg = std::string("Error: socket creation failed : ") + std::strerror(errno);
    return (msg.c_str());
}

const char* Server::BadPass::what() const throw()
{
    return ("ircserv:\nError: password may contains alpha numeric characters.");
}

const char* Server::BadPort::what() const throw()
{
    return ("ircserv:\nError: port out of range (1024-49151).");
}

const char* Server::SetsockoptError::what() const throw()
{
    static std::string msg = std::string("Error in setsockopt function : ") + std::strerror(errno);
    return (msg.c_str());
}

const char* Server::BindError::what() const throw()
{
    static std::string msg = std::string("Error in bind : ") + std::strerror(errno);
    return (msg.c_str());
}

const char* Server::ListenError::what() const throw()
{
    static std::string msg = std::string("Error in listen : ") + std::strerror(errno);
    return (msg.c_str());
}

const char* Server::ReceptionError::what() const throw()
{
    static std::string msg = std::string("Error with reception : ") + std::strerror(errno);
    return (msg.c_str());
}
