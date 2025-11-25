#ifndef REPLY_HPP
# define REPLY_HPP

# include "ft_irc.hpp"
# include "Server.hpp"
# include "Client.hpp"

std::string reply_format(const Server* server, const Client* client, const std::string& code, const std::string& message);

#endif
