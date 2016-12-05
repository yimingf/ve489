// ClientSocket.cpp

#include "ClientSocket.h"

using namespace std;

ClientSocket::ClientSocket()
{
	m_sock = 0;
	memset ( &m_addr, 0, sizeof ( m_addr ) );
}

ClientSocket::ClientSocket(string host_ip, unsigned short port)
{
	m_sock = socket( PF_INET, SOCK_STREAM, 0 );
	if ( m_sock <= 0 )
  	{
  		perror("socket() failed");
    	exit( EXIT_FAILURE );
  	}

  	m_addr.sin_family = PF_INET;
  	inet_pton ( AF_INET, host_ip.c_str(), &m_addr.sin_addr );
	m_addr.sin_port = htons( port );

	if ( connect( m_sock, (struct sockaddr *)&m_addr,
                sizeof( m_addr ) ) < 0 )
  	{
	   	perror( "connect() failed" );
    /*	exit( EXIT_FAILURE );*/
	   	ConnectionFailure e;
    	throw e;
  	}
  	printf( "Connected to %s\n", inet_ntoa( m_addr.sin_addr ) );
}

ClientSocket::~ClientSocket()
{
	close( m_sock );
}

struct peer_info ClientSocket::getPeerInfo()
{
	struct peer_info pi;
	inet_ntop(AF_INET, &m_addr.sin_addr, pi.ip, 16 );
	pi.port = ntohs( m_addr.sin_port );
	return pi; 
}

bool ClientSocket::recv_helper( char* foo, unsigned int len )
{	
	// Assume foo has enough space for len chars
	unsigned int recved = 0;
	int cnt;
	while ( recved < len )
	{
		cnt = ::recv( m_sock, ( foo + recved ) , (len - recved), 0 );
		if ( cnt > 0 )
		{
			recved += (unsigned int) cnt;
			continue;
		}
		if ( cnt == 0 ){
			printf("Socket close while claiming to send %d bytes.\n", len);
			delete[] foo;
			SocketError e;
			throw e;
		}
		if ( cnt < 0 ){
			// Exceeds time out or sock error
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				if (recved == 0) return false;
				printf("Socket Time Out while claiming to send %d bytes.\n", len);
				delete[] foo;
				SocketError e;
				throw e;
			}
			perror("recv() failed");
			delete[] foo;
			SocketError e;
			throw e;
		}
	}
	return true;
}


unsigned int ClientSocket::recv( char* &s )
{
	// delete s outside.
	char* lenC = new char[4] {};

	// set time out
	struct timeval timeout = { TIME_OUT_SEC, TIME_OUT_MS };
	setsockopt( m_sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(struct timeval));

	// First recv the length of packet
	if ( ! recv_helper( lenC, 4 ) ) 
	{
		delete[] lenC;
		return 0;
	}

	unsigned int len = TransCHAR ( lenC );
	s = new char[len + 1] {};
	memmove( s, lenC, 4 );

	delete[] lenC;
	lenC = NULL;

	if ( ! recv_helper( s + 4, len - 4) ){
		printf("Invalid packet!\n");
		delete[] s;
		SocketError e;
		throw e;
	}
	return len;
}

bool ClientSocket::send( const char* s, const unsigned int len )
{
	int status = ::send ( m_sock, s, len, MSG_NOSIGNAL );
  	if ( status == -1 )
    {
    	return false;
    }
  	else
    {
    	return true;
    }
}
