#include "../incs/Command.hpp"

Command::Command() : _raw("")
{
	static t_functions funcs[18] = 
	{
		{"PASS", &Command::_execPass},
		{"KICK", &Command::_execKick},
		{"OPER", &Command::_execOper},
		{"MODE", &Command::_execMode},
		{"NICK", &Command::_execNick},
		{"USER", &Command::_execUser},
		{"JOIN", &Command::_execJoin},
		{"PART", &Command::_execPart},
		{"INVITE", &Command::_execInvite},
		{"TOPIC", &Command::_execTopic},
		{"NAMES", &Command::_execNames},
		{"LIST", &Command::_execList},
		{"PRIVMSG", &Command::_execPrivmsg},
		{"NOTICE", &Command::_execNotice},
		{"QUIT", &Command::_execQuit},
		{"PING", &Command::_execPing},
		{"PONG", &Command::_execPong},
		{"BOT", &Command::_execBot}
	};
	funcptr = funcs;
	std::srand(std::time(NULL));
}

Command::~Command()
{}

Command::Command( const Command& other ) : _raw(other._raw)
{
	_params.operator=(other._params);
}

Command& Command::operator=( const Command& other )
{
	if (this != &other)
	{
		_raw = other._raw;
		_params = other._params;
	}
	return *this;
}

void Command::setParams( const std::string& raw )
{
	_raw = raw;
	_params.clear();
	std::istringstream iss(raw);
	std::string token;
	while (iss >> token)
	{
		if (!token.empty() && token[0] == ':')
		{
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest[0] == ' ')
				rest.erase(0, 1);
			if (rest.empty())
				_params.push_back(token.substr(1) + "");
			else
				_params.push_back(token.substr(1) + " " + rest);
			return;
		}
		_params.push_back(token);
	}
}

void Command::clearParams( void )
{
	_params.clear();
	_raw.clear();
}

const std::vector<std::string>& Command::getParams( void ) const
{
	return _params;
}

const std::string& Command::getRaw( void ) const
{
	return _raw;
}

size_t Command::getSize( void )
{
	return _params.size();
}

void	Command::_execBot( Server* server, Client* client )
{
	if (this->getSize() < 2)
	{
		std::string msg = "PRIVMSG " + client->getNick() + " :/BOT <max value>";
		setParams(msg);
		_execPrivmsg(server, const_cast<Client*>(server->getBot()));
		return ;
	}
	long	nbr = atoi(getParams()[1].c_str());

	if (nbr <= 1 || nbr >= INT32_MAX)
	{
		std::string msg = "PRIVMSG " + client->getNick() + " : your max range must be in range <2 - MAX_INT>";
		setParams(msg);
		_execPrivmsg(server, const_cast<Client*>(server->getBot()));
		return ;
	}

	std::ostringstream	oss;
	
	int	val = std::rand() % nbr + 1;
	oss <<  nbr << " is : " << val;
	std::string msg = "PRIVMSG " + client->getNick() + " :Your random in rage 1 - " + oss.str();
	setParams(msg);
	_execPrivmsg(server, const_cast<Client*>(server->getBot()));
}

void	Command::_execPass( Server* server, Client* client )
{
	if (this->getSize() < 2)
	{
		server->sendToClient("Not enough parameters", client, "461");
		return ;
	}
	std::string passwd = this->getParams()[1];
	if (passwd == server->getPassword())
		client->setPwdAccess(true);
	else
		server->sendToClient("Password incorrect", client, "464");
}

void	Command::_execNick( Server* server, Client* client )
{
	if (this->getSize() < 2)
		return ;
	std::string nick = this->getParams()[1];
	if (!client->setNick(nick, server->getClients()))
	{
		server->sendToClient("* Nickname is already taken", client, "433");
		return ;
	}
	for (size_t i = 0; i < getParams()[1].size(); i++)
	{
		if (!std::isalnum(static_cast<int>(getParams()[1][i])))
		{
			server->sendToClient("* Nickname must be only alnums chars.", client, "433");
			return ;
		}
	}
	if (!client->getName().empty())
	{
		if (!client->getStatus())
			server->sendToClient("* You just registered. Welcome.", client, "001");
		client->setRegistered(true);
	}
}

void	Command::_execUser( Server* server, Client* client )
{
	if (this->getSize() < 2)
	{
		server->sendToClient("* Not enough parameters.", client, "433");
		return ;
	}
	for (size_t i = 0; i < getParams()[1].size(); i++)
	{
		if (!std::isalnum(static_cast<int>(getParams()[1][i])))
		{
			server->sendToClient("* User Name must be only alnums chars.", client, "433");
			return ;
		}
	}
	client->setUserName(this->getParams()[1]);
	if (!client->getNick().empty())
	{
		if (!client->getStatus())
			server->sendToClient("* You just registered. Welcome.", client, "001");
		client->setRegistered(true);
	}
}

void	Command::_execJoin( Server* server, Client* client )
{
	if (this->getSize() < 2)
		return ;
	std::string channel = this->getParams()[1];
	std::string key = "";
	if (this->getSize() >= 3)
		key = this->getParams()[2];
	if (server->addUserToChannel(channel, client, key))
	{
		Channel* ch = server->getChannelByName(channel);
		if (ch)
		{
			std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " JOIN :" + channel + "\r\n";
			for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
				if (*it) (*it)->queueSend(msg);
		}
		if (ch)
		{
			if (ch->getTopic().empty() && ch->getModes().empty())
				server->sendToClient(ch->getName() + " :No topic is set", client, "331");
			else
			{
				std::string	modes;
				if (ch->getModes().empty())
					server->sendToClient(ch->getName() + " :" + ch->getTopic(), client, "332");
				else
					server->sendToClient(ch->getName() + " :" + "[+" + ch->getModes() + "]" + ch->getTopic(), client, "332");
			}
			const size_t MAX_CHUNK = 400;
			std::string chunk;
			std::string names = ch->getUsersStr();
			std::istringstream iss(names);
			std::string nick;
			while (iss >> nick)
			{
				std::string toadd = nick;
				if (!chunk.empty())
					toadd = " " + toadd;
				if (chunk.size() + toadd.size() > MAX_CHUNK)
				{
					std::string reply = "= " + ch->getName() + " :" + chunk;
					server->sendToClient(reply, client, "353");
					chunk.clear();
					if (toadd.size() && toadd[0] == ' ')
						toadd.erase(0, 1);
				}
				if (chunk.empty())
					chunk = toadd;
				else
					chunk += toadd;
			}
			if (!chunk.empty())
			{
				std::string reply = "= " + ch->getName() + " :" + chunk;
				server->sendToClient(reply, client, "353");
			}
			server->sendToClient(" " + ch->getName() + " :End of /NAMES", client, "366");
		}
		return ;
	}
	if (server->createChannel(channel, client))
	{
		if (!key.empty())
		{
			Channel* ch = server->getChannelByName(channel);
			if (ch)
			{
				ch->addMode("k");
				ch->setPwd(key);
			}
		}
		Channel* ch = server->getChannelByName(channel);
		if (ch)
		{
			std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " JOIN " + channel + "\r\n";
			for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
				if (*it) (*it)->queueSend(msg);

			if (ch->getTopic().empty())
				server->sendToClient(ch->getName() + " :No topic is set", client, "331");
			else
			{
				std::string	modes;
				if (ch->getModes().empty())
					ch->getName() + " :" + ch->getTopic();
				else
					ch->getName() + " :" + "[+" + ch->getModes() + "]" + ch->getTopic();
				server->sendToClient(ch->getName() + " :" + "[+" + ch->getModes() + "]" + ch->getTopic(), client, "332");
			}

			const size_t MAX_CHUNK = 400;
			std::string chunk;
			for (std::vector<Client*>::const_iterator jt = ch->getUsers().begin(); jt != ch->getUsers().end(); ++jt)
			{
				if (!*jt)
					continue;
				std::string nick;
				if (ch->getOwner() == (*jt)->getNick() || ch->isOp(*jt))
					nick = "@" + (*jt)->getNick();
				else
					nick = (*jt)->getNick();
				if (!chunk.empty())
					nick = " " + nick;
				if (chunk.size() + nick.size() > MAX_CHUNK)
				{
					std::string reply = "= " + ch->getName() + " :" + chunk;
					server->sendToClient(reply, client, "353");
					chunk.clear();
					if (nick.size() && nick[0] == ' ')
						nick.erase(0, 1);
				}
				if (chunk.empty())
					chunk = nick;
				else
					chunk += nick;
			}
			if (!chunk.empty())
			{
				std::string reply = "= " + ch->getName() + " :" + chunk;
				server->sendToClient(reply, client, "353");
			}
			server->sendToClient(" " + ch->getName() + " :End of /NAMES", client, "366");
		}
		return ;
	}
}

void	Command::_execPart( Server* server, Client* client )
{
	if (this->getParams().size() < 2 || this->getParams()[1].find('#') != 0)
	{
		server->sendToClient("Part: bad arguments", client, "461");
		return ;
	}
	Channel* tmp = server->getChannelByName(getParams()[1]);
	if (!tmp)
	{
		server->sendToClient("No such channel", client, "403");
		return ;
	}
	tmp->rmUser(client);
	if (tmp->getUserCount() == 0)
		server->deleteChannel(tmp);
}

void	Command::_execList( Server* server, Client* client )
{
	for (std::vector<Channel*>::const_iterator it = server->getChannels().begin(); it != server->getChannels().end(); ++it)
	{
		Channel* ch = *it;
		if (!ch)
			continue ;
		std::ostringstream	oss;
		oss << " " << ch->getUserCount() << " :";
		if (!ch->getModes().empty())
			oss << "[+" + ch->getModes() + "]";
		std::string reply = ch->getName() + oss.str() + " " + ch->getTopic();
		server->sendToClient(reply, client, "322");
	}
	server->sendToClient("End of /LIST", client, "323");
}

void	Command::_execTopic( Server* server, Client* client )
{
	if (this->getSize() < 2)
		return ;
	std::string channel = this->getParams()[1];
	Channel* ch = server->getChannelByName(channel);
	if (!ch)
	{
		server->sendToClient("No such channel", client, "403");
		return ;
	}
	if (this->getSize() == 2)
	{
		if (ch->getTopic().empty())
			server->sendToClient(ch->getName() + " :No topic is set", client, "331");
		else
			server->sendToClient(ch->getName() + " :" + ch->getTopic(), client, "332");
		return ;
	}
	std::string newTopic = this->getParams()[2];
	if (ch->hasMode('t'))
	{
		if (ch->getOwner() != client->getNick() && !client->isOp())
		{
			server->sendToClient("You're not channel operator", client, "482");
			return ;
		}
	}
	ch->setTopic(newTopic);
	std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " TOPIC " + channel + " :" + newTopic + "\r\n";
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
	{
		(*it)->queueSend(msg);
	}
}

void	Command::_execOper( Server* server, Client* client )
{
	if (this->getSize() < 2)
	{
		server->sendToClient("Not enough parameters", client, "461");
		return ;
	}
	else if (client->isOp())
	{
		server->sendToClient("You are already an IRC operator", client, "381");
		return ;
	}
	std::string passwd = this->getParams()[1];
	if (passwd == server->getOpPwd())
	{
		client->addOperator();
		server->sendToClient("You are now an IRC operator", client, "381");
	}
	else
		server->sendToClient("Password incorrect", client, "464");
}

void	Command::_execMode( Server* server, Client* client )
{
	if (this->getSize() < 2)
	{
		server->sendToClient("Not enough parameters", client, "461");
		return ;
	}
	std::string channel = this->getParams()[1];
	Channel* ch = server->getChannelByName(channel);
	if (!ch)
	{
		server->sendToClient("No such channel", client, "403");
		return ;
	}
	if (this->getSize() == 2)
	{
		std::string modes = ch->getModes();
		std::string reply = ch->getName() + " ";
		if (!modes.empty())
			reply += modes;
		std::vector<std::string> params;
		if (modes.find('l') != std::string::npos)
		{
			size_t max = ch->getMaxUsers();
			if (max != static_cast<size_t>(-1))
			{
				std::ostringstream oss;
				oss << max;
				params.push_back(oss.str());
			}
		}
		if (modes.find('k') != std::string::npos)
		{
			const std::string &pwd = ch->getPwd();
			if (!pwd.empty())
				params.push_back(pwd);
		}
		for (std::vector<std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
			reply += " " + *it;
		server->sendToClient(reply, client, "324");
		return ;
	}

	std::string modes = this->getParams()[2];
	if (ch->getOwner() != client->getNick() && !ch->isOp(client) && !client->isOp())
	{
		server->sendToClient("You're not channel operator", client, "482");
		return ;
	}
	if (modes.empty() || (modes[0] != '+' && modes[0] != '-'))
	{
		server->sendToClient("Unsupported mode format", client, "472");
		return ;
	}

	size_t paramIdx = 3;
	char curSign = 0;
	std::string appliedModes = "";
	std::vector<std::string> appliedParams;

	for (size_t i = 0; i < modes.size(); ++i)
	{
		char c = modes[i];
		if (c == '+' || c == '-')
		{
			curSign = c;
			if (appliedModes.empty() || appliedModes[appliedModes.length() - 1] == '+' || appliedModes[appliedModes.length() - 1] == '-')
			{
				appliedModes += curSign;
			}
			else if (appliedModes[appliedModes.length() - 1] != curSign)
			{
				appliedModes += curSign;
			}
			continue;
		}
		char flag = c;
		appliedModes += flag;
		if (flag == 'o')
		{
			if (paramIdx >= this->getSize())
			{
				server->sendToClient("Not enough parameters", client, "461");
				break;
			}
			std::string targetNick = this->getParams()[paramIdx++];
			Client* target = server->getClientByNick(targetNick);
			if (!target)
			{
				server->sendToClient("No such nick", client, "401");
				continue;
			}
			if (curSign == '+')
			{
				if (ch->addOp(target))
					ch->addMode("o");
				appliedParams.push_back(targetNick);
			}
			else
			{
				ch->rmOp(target);
				ch->rmMode("o");
				appliedParams.push_back(targetNick);
			}
		}
		else if (flag == 'k')
		{
			if (curSign == '+')
			{
				if (paramIdx >= this->getSize())
				{
					server->sendToClient("Not enough parameters", client, "461");
					break;
				}
				std::string key = this->getParams()[paramIdx++];
				ch->addMode("k");
				ch->setPwd(key);
				appliedParams.push_back(key);
			}
			else
			{
				ch->rmMode("k");
				ch->setPwd("");
			}
		}
		else if (flag == 'l')
		{
			if (curSign == '+')
			{
				if (paramIdx >= this->getSize())
				{
					server->sendToClient("Not enough parameters", client, "461");
					break;
				}
					int limit = atoi(this->getParams()[paramIdx++].c_str());
					if (limit <= 0)
					{
						server->sendToClient("Invalid limit parameter", client, "472");
						continue;
					}
					if (ch->setMaxUsers(static_cast<size_t>(limit)))
					{
						ch->addMode("l");
						std::ostringstream oss;
						oss << limit;
						appliedParams.push_back(oss.str());
					}
				else
				{
					server->sendToClient("Invalid limit (less than current users)", client, "472");
				}
			}
			else
			{
				ch->setMaxUsers((size_t)-1);
				ch->rmMode("l");
			}
		}
		else if (flag == 'i' || flag == 't')
		{
			if (curSign == '+')
				ch->addMode(std::string(1, flag));
			else
				ch->rmMode(std::string(1, flag));
		}
		else
		{
			server->sendToClient("Mode not supported for channel", client, "472");
				if (!appliedModes.empty() && appliedModes[appliedModes.length() - 1] == flag)
					appliedModes.resize(appliedModes.size() - 1);
			continue;
		}
	}
	if (!appliedModes.empty())
	{
		std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " MODE " + channel + " " + appliedModes;
		for (std::vector<std::string>::const_iterator it = appliedParams.begin(); it != appliedParams.end(); ++it)
			msg += " " + *it;
		msg += "\r\n";
		for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
			if (*it) (*it)->queueSend(msg);
	}
	return ;
}

void	Command::_execNames( Server* server, Client* client )
{
	if (this->getSize() >= 2 && !this->getParams()[1].empty() && this->getParams()[1][0] == '#')
	{
		Channel* ch = server->getChannelByName(this->getParams()[1]);
		if (!ch)
		{
			server->sendToClient("No such channel", client, "403");
			return ;
		}
		const size_t MAX_CHUNK = 400;
		std::string chunk;
		std::string names = ch->getUsersStr();
		std::istringstream iss(names);
		std::string nick;
		while (iss >> nick)
		{
			std::string toadd = nick;
			if (!chunk.empty())
				toadd = " " + toadd;
			if (chunk.size() + toadd.size() > MAX_CHUNK)
			{
				std::string reply = "= " + ch->getName() + " :" + chunk;
				server->sendToClient(reply, client, "353");
				chunk.clear();
				if (toadd.size() && toadd[0] == ' ')
					toadd.erase(0, 1);
			}
			if (chunk.empty())
				chunk = toadd;
			else
				chunk += toadd;
		}
		if (!chunk.empty())
		{
			std::string reply = "= " + ch->getName() + " :" + chunk;
			server->sendToClient(reply, client, "353");
		}
		server->sendToClient(" " + ch->getName() + " :End of /NAMES", client, "366");
		return ;
	}
	std::string	chName;
	for (std::vector<Channel*>::const_iterator it = server->getChannels().begin(); it != server->getChannels().end(); ++it)
	{
		Channel* ch = *it;
		if (!ch)
			continue ;
		const size_t MAX_CHUNK = 400;
		std::string chunk;
		std::string names = ch->getUsersStr();
		std::istringstream iss(names);
		std::string nick;
		while (iss >> nick)
		{
			std::string toadd = nick;
			if (!chunk.empty())
				toadd = " " + toadd;
			if (chunk.size() + toadd.size() > MAX_CHUNK)
			{
				std::string reply = "= " + ch->getName() + " :" + chunk;
				server->sendToClient(reply, client, "353");
				chunk.clear();
				if (toadd.size() && toadd[0] == ' ')
					toadd.erase(0, 1);
			}
			if (chunk.empty())
				chunk = toadd;
			else
				chunk += toadd;
		}
		if (!chunk.empty())
		{
			std::string reply = "= " + ch->getName() + " :" + chunk;
			server->sendToClient(reply, client, "353");
		}
		chName = ch->getName();
	}
	server->sendToClient(" " + chName + " :End of /NAMES", client, "366");
}

void	Command::_execPrivmsg( Server* server, Client* client )
{
	if (this->getSize() <= 1)
		return ;
	if (getParams()[1] == "BOT" && getParams().size() > 2)
	{
		std::string	nRaw = getParams()[1] + " :" + getParams()[2];
		setParams(nRaw);
		_execBot(server, client);
		return ;
	}
	std::string target = this->getParams()[1];
	std::string message = "";
	if (this->getSize() >= 3)
		message = this->getParams()[2];
	if (!target.empty() && target[0] == '#')
	{
		if (message.empty())
			return ;
		Channel*	ch = server->getChannelByName(target);
		if (!ch)
			server->sendToClient(":No suck nick/channel", client, "401");
		else
			server->sendToChannel(message, target, client);
		return ;
	}
	Client*	targetClient;
	if (target == "BOT")
		targetClient = const_cast<Client*>(server->getBot());
	else
		targetClient = server->getClientByNick(target);
	if (!targetClient)
		return (server->sendToClient(":No suck nick/channel", client, "401"));
	std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp()
					  + " PRIVMSG " + target + " :" + message + "\r\n";
	std::cout << targetClient->getNick() << std::endl;
	targetClient->queueSend(msg);
}

void	Command::_execNotice( Server* server, Client* client )
{
	if (this->getSize() <= 1)
		return ;
	std::string target = this->getParams()[1];
	std::string message = "";
	if (this->getSize() >= 3)
		message = this->getParams()[2];
	if (!target.empty() && target[0] == '#')
	{
		if (message.empty())
			return ;
		server->sendToChannel(message, target, client);
		return ;
	}
	Client* targetClient = server->getClientByNick(target);
	if (!targetClient)
		return ;
	std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp()
		+ " NOTICE " + target + " :" + message + "\r\n";
	targetClient->queueSend(msg);
}

void	Command::_execQuit( Server* server, Client* client )
{
	server->removeClient(client);
}

void	Command::_execPing( Server* server, Client* client )
{
	(void)server;
	std::string msg = "PONG";
	if (getParams().size() >= 2 && !getParams()[1].empty())
		msg += " :" + getParams()[1];
	server->sendToClient(msg, client, "");
}

void	Command::_execPong( Server* server, Client* client )
{
	if (!client->isPinged())
		return ;
	(void)server;
	if (!client->isPinged() || getParams().size() != 2)
		return ;
	else if (atoi(getParams()[1].c_str()) == client->getPingCode())
	{
		client->setPinged(false);
		client->setPingCode(-1);
	}
}

void	Command::_execInvite( Server* server, Client* client )
{
	if (this->getSize() < 3)
	{
		server->sendToClient("Not enough parameters", client, "461");
		return ;
	}
	std::string targetNick = this->getParams()[1];
	std::string channel = this->getParams()[2];
	Channel* ch = server->getChannelByName(channel);
	if (!ch)
	{
		server->sendToClient("No such channel", client, "403");
		return ;
	}
	bool inviterOn = false;
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
		if (*it == client)
		{
			inviterOn = true;
			break;
		}
	if (!inviterOn)
	{
		server->sendToClient("You're not on that channel", client, "442");
		return ;
	}
	if (ch->getOwner() != client->getNick() && !ch->isOp(client) && !client->isOp())
	{
		server->sendToClient("You're not channel operator", client, "482");
		return ;
	}
	Client* target = server->getClientByNick(targetNick);
	if (!target)
	{
		server->sendToClient("No such nick", client, "401");
		return ;
	}
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
		if (*it == target)
		{
			server->sendToClient("User is already on channel", client, "443");
			return ;
		}
	ch->addInvite(targetNick);
	std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " INVITE " + targetNick + " " + channel + "\r\n";
	target->queueSend(msg);
	server->sendToClient("Inviting " + targetNick + " to " + channel, client, "341");
}

void	Command::_execKick( Server* server, Client* client )
{
	if (this->getSize() < 3)
	{
		server->sendToClient("Not enough parameters", client, "461");
		return ;
	}
	std::string channel = this->getParams()[1];
	std::string targetNick = this->getParams()[2];
	std::string reason = "";
	if (this->getSize() >= 4)
		reason = this->getParams()[3];
	Channel* ch = server->getChannelByName(channel);
	if (!ch)
	{
		server->sendToClient("No such channel", client, "403");
		return ;
	}
	bool callerOn = false;
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
		if (*it == client)
		{
			callerOn = true;
			break;
		}
	if (!callerOn)
	{
		server->sendToClient("You're not on that channel", client, "442");
		return ;
	}
	Client* target = server->getClientByNick(targetNick);
	if (!target)
	{
		server->sendToClient("No such nick", client, "401");
		return ;
	}
	bool targetOn = false;
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
		if (*it == target)
		{
			targetOn = true;
			break;
		}
	if (!targetOn)
	{
		server->sendToClient("User not in channel", client, "441");
		return ;
	}
	if (ch->getOwner() != client->getNick() && !ch->isOp(client) && !client->isOp())
	{
		server->sendToClient("You're not channel operator", client, "482");
		return ;
	}
	std::string msg = ":" + client->getNick() + "!" + client->getName() + "@" + client->getIp() + " KICK " + channel + " " + targetNick;
	if (!reason.empty())
		msg += " :" + reason;
	msg += "\r\n";
	for (std::vector<Client*>::const_iterator it = ch->getUsers().begin(); it != ch->getUsers().end(); ++it)
		if (*it)
			(*it)->queueSend(msg);
	ch->rmUser(target);
	if (ch->getUserCount() == 0)
		server->deleteChannel(ch);
}

void	Command::execute( Server* server, Client* client, const std::string& raw )
{
	Command	cmd;

	cmd.setParams(raw);
	if (!client || cmd.getParams().empty())
		return ;
	if (cmd.getParams()[0] != "PONG" && client->isPinged())
		return ;
	client->setLastAction();
	if (cmd.getParams()[0] == "QUIT")
		return (cmd._execQuit(server, client));
	if (cmd.getSize() > 1 && cmd.getParams()[0] == "CAP" && cmd.getParams()[1] == "LS")
		return (server->sendToClient("CAP * LS :", client, ""), cmd.clearParams());
	if (!client->getPwdAccess() && cmd.getSize() > 1 && cmd.getParams()[0] != "PASS")
		return (server->sendToClient("* Please set server password.", client, "451"), cmd.clearParams());
	if (cmd.getParams()[0] != "NICK" && cmd.getParams()[0] != "USER" && (!client->getPwdAccess() && cmd.getParams()[0] != "PASS"))
		return (server->sendToClient("* Register before exec any command", client, "451"), cmd.clearParams());
	if ((!client->getStatus() || !client->getPwdAccess()) && cmd.getParams()[0] != "PASS" && cmd.getParams()[0] != "USER" && cmd.getParams()[0] != "NICK")
		return (server->sendToClient("* Register before exec any command", client, "451"), cmd.clearParams());
	size_t	idx = 0;
	while (idx < 18)
	{
		if ((cmd.funcptr)[idx].name == cmd.getParams()[0])
			break ;
		idx++;
	}
	if (idx >= 18)
	{
		cmd.clearParams();
		return ;
	}
	((cmd.*cmd.funcptr[idx].f)(server, client));
	cmd.clearParams();
}