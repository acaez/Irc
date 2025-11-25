#include "../incs/Reply.hpp"

static bool reply_isDigits(const std::string& s)
{
    if (s.empty())
        return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    return true;
}

std::string reply_format(const Server* server, const Client* client, const std::string& code, const std::string& message)
{
    std::string srv;
    if (server)
        srv = server->getIp();
    else
        srv = std::string("server");

    std::ostringstream oss;
    if (code.empty())
    {
        oss << ":" << srv << " " << message << "\r\n";
        return oss.str();
    }
    std::string codeOut = code;
    if (reply_isDigits(code))
    {
        if (codeOut.size() < 3)
            codeOut = std::string(3 - codeOut.size(), '0') + codeOut;
    }
    oss << ":" << srv << " " << codeOut;
    if (client && !client->getNick().empty())
        oss << " " << client->getNick();
	else
		oss << " ***";
    if (!message.empty())
        oss << " " << message;
    oss << "\r\n";
    return oss.str();
}
