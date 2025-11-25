#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# include <iostream>
# include <string>
# include <cstdlib>
# include <vector>
# include <map>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <poll.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <unistd.h>
# include <stdint.h>
# include <sstream>
# include <algorithm>
# include <ctime>
# include <sstream>

# define PING_TIME 90
# define LOGOUT_TIME 120
# define REGISTRATION_TIME 30
# define OUTBUF_LIMIT 65536

#endif