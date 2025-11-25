#include "../incs/Client.hpp"

Client::Client() : _fd(-1), _ip(""), _port(-1), _isOperator(false), _nick(""), _name(""), _registered(false), _pwdAccess(false), _isBan(false), _isPinged(false), _pingCode(-1), _isBot(false)
{
    _lastAction = std::time(NULL);
}

Client::Client( Fd fd ) : _fd(fd), _ip(""), _port(-1), _isOperator(false), _nick(""), _name(""), _registered(false), _pwdAccess(false), _isBan(false), _isPinged(false), _pingCode(-1), _isBot(false)
{
    _lastAction = std::time(NULL);
}

Client::Client( Fd fd , const std::string& nick ) : _fd(fd), _ip(""), _port(-1), _isOperator(false), _nick(nick), _name(""), _registered(false), _pwdAccess(false), _isBan(false), _isPinged(false), _pingCode(-1), _isBot(false)
{
    _lastAction = std::time(NULL);
}

Client::~Client()
{}

Client::Client( const Client& other )
{
    *this = other;
}

Client& Client::operator=( const Client& other )
{
    if (this != &other)
    {
        this->_fd.operator=(other._fd);
        this->_ip = other._ip;
        this->_port = other._port;
		this->_isOperator = other._isOperator;
		this->_nick = other._nick;
		this->_name = other._name;
        this->_registered = other._registered;
        this->_pwdAccess = other._pwdAccess;
        this->_isBan = other._isBan;
        this->_lastAction = other._lastAction;
        this->_pingCode = other._pingCode;
        this->_isBot = other._isBot;
    }
    return (*this);
}

bool    Client::queueSend( const std::string& data )
{
    int fd = this->_fd.getFd();
    if (fd < 0)
    {
        if (this->_outBuf.size() + data.size() > OUTBUF_LIMIT)
            return (false);
        this->_outBuf += data;
        return (true);
    }
    if (this->_outBuf.size() + data.size() > OUTBUF_LIMIT)
        return (false);
    this->_outBuf += data;
    return (true);
}

int Client::getFd( void ) const
{
    return (this->_fd.getFd());
}

void    Client::setFd( Fd fd )
{
    if (_fd.getFd() >= 0)
        close(_fd.getFd());
    this->_fd = fd;
}

void    Client::setPort( int port )
{
    this->_port = port;
}

void    Client::setIp( const std::string& ip )
{
    this->_ip = ip;
}

const std::string&  Client::getIp( void ) const
{
    return (this->_ip);
}

int  Client::getPort( void ) const
{
    return (this->_port);
}

void    Client::addOperator( void )
{
    this->_isOperator = true;
}

void    Client::removeOperator( void )
{
    this->_isOperator = false;
}

bool    Client::isOp( void ) const
{
    return (this->_isOperator);
}

bool    Client::setNick( const std::string newNick, const std::map<int, Client*>& clients )
{
    if (newNick == "BOT")
        return (false);
    else
    {
        for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second->getNick() == newNick)
                return (false);
        }
        this->_nick = newNick;
        return (true);
    }
}

void    Client::setUserName( const std::string& newName )
{
    this->_name = newName;
}

const std::string& Client::getName( void ) const
{
    return (this->_name);
}

const std::string& Client::getNick( void ) const
{
    return (this->_nick);
}

bool    Client::getStatus( void ) const
{
    return (this->_registered);
}

void    Client::setRegistered( bool status )
{
    this->_registered = status;
}

bool    Client::getPwdAccess( void )
{
    return (this->_pwdAccess);
}

void    Client::setPwdAccess( bool access )
{
    this->_pwdAccess = access;
}

bool    Client::getBanStatus( void ) const
{
    return (this->_isBan);
}

void    Client::setBanStatus( bool status )
{
    this->_isBan = status;
}

void    Client::setLastAction( void )
{
    this->_lastAction = std::time(NULL);
}

std::time_t  Client::getLastAction( void ) const
{
    return (_lastAction);
}

void    Client::setPinged( bool status )
{
    this->_isPinged = status;
}

bool    Client::isPinged( void )
{
    return (this->_isPinged);
}

void    Client::setPingCode( int code )
{
    this->_pingCode = code;
}

int Client::getPingCode( void )
{
    return (this->_pingCode);
}

const std::string& Client::getOutBuf( void ) const
{
    return (this->_outBuf);
}

void Client::popOutBuf( size_t n )
{
    if (n >= _outBuf.size())
        _outBuf.clear();
    else
        _outBuf.erase(0, n);
}

bool    Client::isBot( void )
{
    return (this->_isBot);
}

void    Client::setBot( bool status )
{
    this->_isBot = status;
}