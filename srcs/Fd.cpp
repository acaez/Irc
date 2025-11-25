#include "../incs/Fd.hpp"

Fd::Fd() : _fd(-1)
{}

Fd::Fd( int fd )
{
    _fd = fd;
}

Fd::~Fd()
{
    if (_fd >= 0)
    {
        close(_fd);
        _fd = -1;
    }
}

Fd::Fd( const Fd& other )
{
    _fd = other._fd;
    const_cast<Fd&>(other)._fd = -1;
}

Fd& Fd::operator=( const Fd& other )
{
    if (this != &other)
    {
        if (_fd >= 0)
        {
            close(_fd);
            _fd = -1;
        }
        if (other._fd >= 0)
        {
            _fd = other._fd;
            const_cast<Fd&>(other)._fd = -1;
        }
        else
            _fd = -1;
    }
    return (*this);
}

int Fd::getFd( void ) const
{
    return (_fd);
}
