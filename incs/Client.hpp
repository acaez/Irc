#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"
# include "Fd.hpp"

class	Channel;

class   Client
{
    public:
        Client();
        Client( Fd fd );
        Client( Fd fd , const std::string& nick );
        ~Client();
        Client( const Client& other );
        Client&     operator=( const Client& other );
    
        void                setFd( Fd fd );
        void                setPort( int port );
        void                setIp( const std::string& ip );
        bool                setNick( const std::string newNick, const std::map<int, Client*>& clients );
        void                setUserName( const std::string& newName );
        void                setRegistered( bool status );
        void                setPwdAccess( bool access );
        void                setBanStatus( bool status );
        void                setLastAction( void );
        void                setPinged( bool status );
        void                setPingCode( int code );
		bool                queueSend( const std::string& data );
		const std::string&  getOutBuf( void ) const;
		void                popOutBuf( size_t n );
        void                setBot( bool status );
        
        
        bool                isBot( void );
        int                 getPingCode( void );
        bool                isPinged( void );
        std::time_t         getLastAction( void ) const;
        bool                getPwdAccess( void );
        int                 getFd( void ) const;
        const std::string&  getIp( void ) const;
        int                 getPort( void ) const;
        const std::string&  getNick( void ) const;
        const std::string&  getName( void ) const;
        bool                isOp( void ) const;
        bool                getStatus( void ) const;
        bool                getBanStatus( void ) const;
        void                addOperator( void );
        void                removeOperator( void );

    private:
		Fd         			    _fd;
        std::string             _ip;
        int                     _port;
		bool        			_isOperator;
        std::string 			_nick;
        std::string 			_name;
		std::string				_realname;
		std::string				_mode;
        bool                    _registered;
        bool                    _pwdAccess;
        bool                    _isBan;
        std::time_t             _lastAction;
        bool                    _isPinged;
        int                     _pingCode;
    	std::string             _outBuf;
        bool                    _isBot;
};

#endif
