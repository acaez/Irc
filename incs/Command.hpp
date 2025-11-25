#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "ft_irc.hpp"
# include "Server.hpp"

class	Client;
class	Channel;
class	Server;

class	Command
{
	public:
		Command();
		~Command();
		Command( const Command& other );
		Command&	operator=( const Command& other );
		static void						execute( Server* server, Client* client, const std::string& raw );
		void							parsing( void );

		void							setParams( const std::string& raw );
		void							clearParams( void );

		const std::vector<std::string>&	getParams( void ) const;
		const std::string&				getRaw( void ) const;
		size_t							getSize( void );

	private:
		std::string					_raw;
		std::vector<std::string>	_params;
		void					_execPass( Server* server, Client* client );
		void					_execNick( Server* server, Client* client );
		void					_execUser( Server* server, Client* client );
		void					_execJoin( Server* server, Client* client );
		void					_execPart( Server* server, Client* client );
		void					_execTopic( Server* server, Client* client );
		void					_execMode( Server* server, Client* client );
		void					_execOper( Server* server, Client* client );
		void					_execNames( Server* server, Client* client );
		void					_execList( Server* server, Client* client );
		void					_execPrivmsg( Server* server, Client* client );
		void					_execNotice( Server* server, Client* client );
		void					_execWho( Server* server, Client* client );
		void					_execQuit( Server* server, Client* client );
		void					_execPing( Server* server, Client* client );
		void					_execPong( Server* server, Client* client );
		void					_execInvite( Server* server, Client* client );
		void					_execKick( Server* server, Client* client );
		void					_execBot( Server* server, Client* client );

		typedef struct s_functions
		{
			std::string name;
			void		(Command::*f)( Server* server, Client* client);
		}	 t_functions;
		t_functions	*funcptr;
};

#endif