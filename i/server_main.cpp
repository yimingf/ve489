// server_main.cpp

#include "global.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Server.h"
#include "Packet.h"

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int main()
{
	try
	{
		Server server;
		ServerSocket* serverSock = new ServerSocket( SERVER_PORT );
		std::cout << "Listening to uploaders and downloaders at Port "
			<< SERVER_PORT << ".\n";

		// The server is always on..
		while (1)
		{
			ClientSocket* inComingClient = serverSock -> accept();
			std::cout << "Received connection from " 
				<< inComingClient -> getPeerInfo().ip << ".\n";
			char* Request;
			unsigned int recvLen = inComingClient -> recv(
				Request );
			if ( recvLen == 0 )
			{
				delete inComingClient;
				std::cout << "Connection Time Out..\n";
				continue; // Just drop the line
			} 

			MessageID type = (MessageID) Request[4];
			if ( type == torFILE ){
				// This is a .torrent file uploaded..

				char* mesToUploader = new char[80] {};
				server.onReceiveTorrent( Request, mesToUploader );
				std::cout << "Received .torrent file REQ.. Sent back "
					<< (unsigned int) type << ".\n";
				if ( !inComingClient -> send(
					mesToUploader, TransCHAR( mesToUploader ) ) )
				{
					delete 		mesToUploader;
					delete 		Request;
					delete 		inComingClient;
					ErrorSend 	e;
					throw 		e;
				}
				delete Request;
				delete mesToUploader;

				// Then recv the uploaders' comfirm.
				unsigned int recvLen = inComingClient -> recv(
					Request );
				if ( recvLen == 0 )
				{
					server.deleteLatestFile();
					std::cout << "[server] The latest uploading file disgarded.\n";
					delete inComingClient;
					continue; // Just drop the line
				} 
				MessageID type2 = (MessageID) Request[4];
				if ( type2 != ACK )
				{
					server.deleteLatestFile();
					std::cout << "[server] The latest uploading file disgarded.\n";
				}
				delete inComingClient;
				delete Request;
				continue; // To serve next req.
			}
			else if ( type == REQ ){
				char* torList = new char[2000] {};

				server.onRequestTorrentList( Request, torList );
				std::cout << "[server] Recved torList REQ from downloader..\n";
				if ( !inComingClient -> send(
					torList, TransCHAR( torList ) ) )
				{
					delete 		torList;
					delete 		Request;
					delete 		inComingClient;
					ErrorSend 	e;
					throw 		e;
				}
				delete torList;
				delete Request;
				delete inComingClient;
				continue;
			}
			else if( type == REQFile ){
				char* torFile = new char[TORRENT_MAX_SIZE] {};
				server.onRequestTorrentSpecified( Request, torFile );
				std::cout << "[server] Recved torFile REQ form downloader..\n";
				if ( !inComingClient -> send(
					torFile, TransCHAR( torFile ) ) )
				{
					delete 		torFile;
					delete 		Request;
					delete 		inComingClient;
					ErrorSend 	e;
					throw 		e;
				}
				delete torFile;
				delete Request;
				delete inComingClient;
				continue;
			}
			else {
				// Unexpected message..
				std::cout << "Recved unexpected message. Line dropped.\n";
				delete Request;
				delete inComingClient;
				continue;
			}
		} 

	// We never get here..
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