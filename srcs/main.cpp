#include "../incs/ft_irc.hpp"
#include "../incs/Server.hpp"
#include "../incs/Client.hpp"

int main( int ac, char **av )
{
    if (ac != 3)
    {
        std::cerr << "Try with a good prompt:\n./ircserv <port> <password>" << std::endl;
        return (1);
    }
    try
    {
		Server	ircserv;

        ircserv.start(av[1], av[2]);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return (0);
}