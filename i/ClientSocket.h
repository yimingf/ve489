// ClientSocket.h

#ifndef __CLIENTSOCKET_H__
#define __CLIENTSOCKET_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include "global.h"

using namespace std;

class ClientSocket
{
protected:

	bool recv_helper (char* foo, unsigned int len);

public:

	int m_sock;
	struct sockaddr_in m_addr;

	ClientSocket();
	ClientSocket(string host_ip, unsigned short port);
	~ClientSocket();

	struct peer_info getPeerInfo();
	unsigned int recv( char* &s );
	bool send( const char* s, const unsigned int len );
};

#endif

