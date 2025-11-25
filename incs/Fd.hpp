#ifndef FD_HPP
# define FD_HPP

# include <unistd.h>
# include <iostream>

class   Fd
{
    public:
        Fd();
        Fd( int fd );
        ~Fd();
        Fd( const Fd& other );
        Fd& operator=( const Fd& other );
        int getFd( void ) const;

    private:
        int _fd;  
};

#endif