#include "../incs/Channel.hpp"
#include "../incs/Reply.hpp"

bool    isModeChar( char c );

Channel::Channel() : _name("#default"), _pwd(""), _topic(""), _maxUsers(0), _userCount(0), _modes(""), _owner("")
{}

Channel::~Channel()
{
    std::cout << this->getName() << " destroyed" << std::endl;
    _users.clear();
}

Channel::Channel( const std::string& name ) : _name(name), _pwd(""), _topic(""), _maxUsers(0), _userCount(0), _modes(""), _owner("")
{
	std::cout << name << " channel created" << std::endl;
}

Channel::Channel( const Channel& other )
{
    *this = other;
}

Channel&    Channel::operator=( const Channel& other )
{
    if (this != &other)
    {
        _name = other._name;
        _users.clear();
        _pwd = other._pwd;
        for (std::vector<Client*>::const_iterator it = other._users.begin(); it != other._users.end(); ++it)
            _users.push_back(*it); 
        _topic = other._topic;
        _maxUsers = other._maxUsers;
        _userCount = other._userCount;
        _modes = other._modes;
        _owner = other._owner;
    }
    return (*this);
}

bool    Channel::operator==(const Channel& other )
{
    return (this->getName() == other.getName());
}

const std::string&  Channel::getName( void ) const
{
    return (_name);
}

const std::vector<Client*>& Channel::getUsers( void ) const
{
    return (_users);
}

const std::string&  Channel::getPwd( void ) const
{
    return (_pwd);
}

void    Channel::setPwd( const std::string& pwd )
{
    this->_pwd = pwd;
}

bool    Channel::setMaxUsers( size_t max )
{
    if (this->getUserCount() > max)
        return (false);
    this->_maxUsers = max;
    return (true);
}

void    Channel::setTopic( const std::string& topic )
{
    this->_topic = topic;
}

void    Channel::increaseUserNumber( void )
{
    this->_userCount++;
}

void    Channel::decreaseUserNumber( void )
{
    if (this->_userCount > 0)
        this->_userCount--;
}

size_t  Channel::getUserCount( void ) const
{
    return (this->_userCount);
}

std::string  Channel::getUserCountStr( void ) const
{
    std::ostringstream  oss;

    oss << this->_userCount;
    return (oss.str());
}

size_t  Channel::getMaxUsers( void ) const
{
    return (this->_maxUsers);
}

const std::string&  Channel::getTopic( void )
{
    return (_topic);
}

const std::string&  Channel::getModes( void ) const
{
    return (this->_modes);
}

bool    Channel::hasMode( char m ) const
{
    return (_modes.find(m) != std::string::npos);
}

bool    Channel::addUser( Client* client )
{
    if (!client)
        return (false);
    for (std::vector<Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
    {
        if (*it == client)
            return (false);
    }
    _users.push_back(client);
    this->increaseUserNumber();
    return (true);
}

void    Channel::rmUser( Client* client )
{    
    if (!client)
        return ;
    std::vector<Client*>::iterator  it = std::find(_users.begin(), _users.end(), client);
    if (it == _users.end())
        return ;

    std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " PART " + getName() + "\r\n";
    for (std::vector<Client*>::const_iterator cit = _users.begin(); cit != _users.end(); ++cit)
    {
        if (*cit)
            (*cit)->queueSend(msg);
    }

    _users.erase(it);
    this->decreaseUserNumber();
    if (this->getOwner() == client->getNick())
        return ;
}

void    Channel::addMode( const std::string& modes )
{
    for(size_t i = 0; i < modes.size(); ++i)
    {
        size_t  pos = _modes.find(modes[i]);
        if (isModeChar(modes[i]) && pos == std::string::npos)
        {
            _modes += modes[i];
            std::sort(_modes.begin(), _modes.end());
        }
    }
}

void    Channel::rmMode( const std::string& modes )
{
    for (size_t i = 0; i < modes.size(); ++i)
    {
        size_t  pos = _modes.find(modes[i]);
        if (pos != std::string::npos)
            _modes.erase(pos);
    }
}

void    Channel::setOwner( Client* client )
{
    this->_owner = client->getNick();
}

const std::string&  Channel::getOwner( void ) const
{
    return (this->_owner);
}

bool    Channel::addOp( Client* client )
{
    if (!client)
        return (false);
    for (std::vector<Client*>::const_iterator it = _ops.begin(); it != _ops.end(); ++it)
    {
        if (*it == client)
            return (false);
    }
    _ops.push_back(client);
    return (true);
}

void    Channel::rmOp( Client* client )
{
    if (!client)
        return ;
    std::vector<Client*>::iterator it = std::find(_ops.begin(), _ops.end(), client);
    if (it == _ops.end())
        return ;
    _ops.erase(it);
}

bool    Channel::isOp( Client* client ) const
{
    if (!client)
        return (false);
    for (std::vector<Client*>::const_iterator it = _ops.begin(); it != _ops.end(); ++it)
    {
        if (*it == client)
            return (true);
    }
    return (false);
}

bool    Channel::isInvited( const std::string& nick ) const
{
    for (std::vector<std::string>::const_iterator it = _invites.begin(); it != _invites.end(); ++it)
    {
        if (*it == nick)
            return (true);
    }
    return (false);
}

void    Channel::addInvite( const std::string& nick )
{
    if (nick.empty())
        return ;
    if (isInvited(nick))
        return ;
    _invites.push_back(nick);
}

void    Channel::rmInvite( const std::string& nick )
{
    std::vector<std::string>::iterator it = std::find(_invites.begin(), _invites.end(), nick);
    if (it == _invites.end())
        return ;
    _invites.erase(it);
}

std::string  Channel::getUsersStr( void ) const
{
    if (_users.empty())
        return (std::string());
    std::string list;

    for (size_t i = 0; i < _users.size(); ++i)
    {
        if (std::find(_ops.begin(), _ops.end(), _users[i]) != _ops.end())
            list += "@";
        list += _users[i]->getNick();
        if (i + 1 < _users.size())
            list += " ";
    }
    return (list);
}