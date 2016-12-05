// tracker_main.cpp

#include "global.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Tracker.h"

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int main()
{
	try
	{
		Tracker tracker;
		ServerSocket* trackerSock = new ServerSocket( TRACKER_PORT );
		std::cout << "[tracker] Listening to uploaders and downloaders at Port "
			<< TRACKER_PORT << ".\n";

		// The tracker is always on..
		while (1)
		{
			ClientSocket* inComingClient = trackerSock -> accept();
			std::cout << "[tracker] Received connection from " 
				<< inComingClient -> getPeerInfo().ip << ".\n";
			char* Request;
			unsigned int recvLen = inComingClient -> recv(  Request );
			if ( recvLen == 0 )
			{
				delete inComingClient;
				std::cout << "Connection Time Out..\n";
				continue; // Just drop the line
			}

			char* Response = new char[2000] {};
			try {
				tracker.onPeerRegister( Request, inComingClient, Response );
			}
			catch ( EmptyTracker ){
			// Unexpected mes form peer.
				std::cout << "recved unexpected message. Line dropped.\n";
				delete Request;
				delete Response;
				delete inComingClient;
			}
			if ( !inComingClient -> send(
				Response, TransCHAR( Response ) ) )
			{
				delete Response;
				delete Request;
				delete inComingClient;
				ErrorSend e;
				throw e;
			}
			delete Response;
			delete Request;
			delete inComingClient;

			continue;
		}

		// We will never get here..
		return EXIT_SUCCESS;
	}
	
	catch( EmptyPacket )
	{
		std::cout << "Incomplete Packet when Constructing.\n";
		return EXIT_FAILURE;
	}
	catch (ErrorSend){
		std::cout << "Error send.\n";
		return EXIT_FAILURE;		
	}
	catch ( SocketError )
	{
		std::cout << "Socket Error.\n";
		return EXIT_FAILURE;
	}
}