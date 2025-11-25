#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"
# include "Client.hpp"

class   Channel
{
    private:
        std::vector<Client*>    _users;
        std::string             _name;
        std::string             _pwd;
        std::string             _topic;
        size_t                  _maxUsers;
        size_t                  _userCount;
        std::string             _modes;
        std::string             _owner;
        std::vector<Client*>    _ops;
        std::vector<std::string> _invites;

    public:
        Channel();
        ~Channel();
        Channel( const std::string& name );
        Channel( const Channel& other );
        Channel&    operator=( const Channel& other );
        bool        operator==(const Channel& other );

        // void                        sendClients( const std::string& msg );
        // void                        addClient( Client* newUser );
        void                            setTopic( const std::string& topic );
        void                            setPwd( const std::string& pwd );
        void                            setOwner( Client* client );
        bool                            setMaxUsers( size_t max );
        void                            increaseUserNumber( void );
        void                            decreaseUserNumber( void );

        size_t                          getUserCount( void ) const;
        std::string                     getUserCountStr( void ) const;
        size_t                          getMaxUsers( void ) const;
        const std::string&              getTopic( void );
        const std::string&              getName( void ) const;
        const std::vector<Client*>&     getUsers( void ) const;
        const std::string&              getPwd( void ) const;
        const std::string&              getOwner( void ) const;
    	const std::string&              getModes( void ) const;
    	bool                            hasMode( char m ) const;
        std::string                     getUsersStr( void ) const;

        bool                            addUser( Client* client );
        void                            rmUser( Client* client );
        void                            addMode( const std::string& modes );
        void                            rmMode( const std::string& modes );
    bool                            addOp( Client* client );
    void                            rmOp( Client* client );
    bool                            isOp( Client* client ) const;
    bool                            isInvited( const std::string& nick ) const;
    void                            addInvite( const std::string& nick );
    void                            rmInvite( const std::string& nick );
};

#endif