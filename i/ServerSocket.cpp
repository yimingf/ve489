// ServerSocket.cpp

#include "ServerSocket.h"
#include "ClientSocket.h"

using namespace std;

ServerSocket::ServerSocket( unsigned short port )
{
	m_sock = socket( PF_INET, SOCK_STREAM, 0 );
	if ( m_sock <= 0 )
  	{
  		perror("socket() failed");
    	exit( EXIT_FAILURE );
  	}

  	m_addr.sin_family = PF_INET;
  	m_addr.sin_addr.s_addr = INADDR_ANY;
  	m_addr.sin_port = htons( port );

  	int len = sizeof( m_addr );
  	if ( bind( m_sock, (struct sockaddr *)&m_addr, len ) < 0 )
  	{
  	  perror("bind() failed");
 	    close( m_sock );
 	    exit( EXIT_FAILURE );
	}

	listen( m_sock, 10 );
	printf( "Listening on port %d...\n", port );
}

ServerSocket::~ServerSocket()
{
	close( m_sock );
}

unsigned short ServerSocket::getListeningPort()
{
  return ntohs( m_addr.sin_port );
}

ClientSocket* ServerSocket::accept()
{
	ClientSocket* newsock = new ClientSocket;
	int addr_length = sizeof ( newsock->m_addr );
	newsock->m_sock = ::accept( m_sock, (struct sockaddr *)&newsock->m_addr,
                          (socklen_t*)&addr_length );
	if ( newsock->m_sock <= 0 ){
		perror( "accept() failed" );
		close( m_sock );
		exit( EXIT_FAILURE );
	}
  char client_ip[20];
  inet_ntop(AF_INET, &m_addr.sin_addr, client_ip, 16 );
  printf( "Received incoming connection from %s\n", client_ip);

	return newsock;
}