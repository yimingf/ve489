// ServerSocket.h

#ifndef __SERVERSOCKET_H__
#define __SERVERSOCKET_H__

#include "ClientSocket.h"
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

using namespace std;

class ServerSocket
{
protected:
	int m_sock;

	struct sockaddr_in m_addr;

public:
	//ServerSocket();

	ServerSocket( unsigned short port = 0 );

	~ServerSocket();

	unsigned short getListeningPort();

	ClientSocket* accept();
	// Invoke new in this method, caller should delete
	// the allocated ClientSocket object
};

#endif
