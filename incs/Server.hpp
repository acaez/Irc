#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"
# include "Client.hpp"
# include "Fd.hpp"
# include "Command.hpp"
# include "Channel.hpp"

bool	isStrAlnum( const std::string& str );

class   Server
{
	private:
		int         					_port;
		std::string						_ip;
		Fd								_serverFd;
		int								_opt;
		std::string 					_password;
		struct sockaddr_in				_address;
		std::map<int, Client*>			_clients;
		std::vector<Channel*>			_channels;
		std::clock_t					_start;
		bool							_running;
		const std::string				_opPwd;
		Client							_bot;

    public:
        Server();
        ~Server();
        Server( const Server& other );
        Server&     					operator=( const Server& other );
        void        					init( const std::string& argOne, const std::string& argTwo );
		void        					start( const std::string& argOne, const std::string& argTwo );
        void        					setPassword( const std::string& password );
        void        					setPort( int port );
		void							setFd( Fd fd );
        const std::string&				getPassword( void ) const;
        int         					getPort( void ) const;
		int								getFd( void ) const;
		Client*							getClientByNick( const std::string& nick ) const;
		Client*							getClientByFd( int fd ) const;
		const std::string&				getIp( void ) const;
		const std::vector<Channel*>&	getChannels( void ) const;
		const std::map<int, Client*>&	getClients( void ) const;
		const std::string&				getOpPwd( void ) const;
		const Client*					getBot( void ) const;
		void							addClient( Client* client );
		void							removeClient( Client* client );
		void							removeFd( int fd );
		Channel*						getChannelByName( const std::string& channel );
		void							initSocket( void );
		void							bindSocket( void );
		void							listenSocket( void );
		void							pollLoop( void );
		void							sendToClient( const std::string& raw, const Client* client, const std::string& code );
		void							sendToChannel( const std::string& raw, const std::string& channel, Client* sender );
		bool							createChannel( const std::string& channel, Client* client );
		bool							addUserToChannel( const std::string& channel, Client* client, const std::string& key = "" );
		void							deleteChannel( Channel *chan );
		void							checkClientsLastAction( void );

		/* --- exceptions ---*/

		class   pollError : public std::exception
        {
			public:
				const char* what() const throw();
        };

        class   BadPort : public std::exception
        {
			public:
				const char* what() const throw();
        };

        class   BadPass : public std::exception
        {
			public:
				const char* what() const throw();
        };

		class	SocketCreationError : public std::exception
		{
			public:
				const char* what() const throw();
		};

		class	SetsockoptError : public std::exception
		{
			public:
				const char* what() const throw();
		};

		class	BindError : public std::exception
		{
			public:
				const char* what() const throw();
		};

		class	ListenError : public std::exception
		{
			public:
				const char* what() const throw();
		};

		class	ReceptionError : public std::exception
		{
			public:
				const char* what() const throw();
		};
};

#endif
