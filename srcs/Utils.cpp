#include "../incs/ft_irc.hpp"

bool    isModeChar( char c )
{
    return (c == 'i' || c == 't' || c =='k' || c == 'o' || c == 'l');
}

bool	isStrAlnum( const std::string& str )
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(str[i])))
			return (false);
	}
	return (true);
}