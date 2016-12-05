// client_main.cpp

#include "global.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Peer.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>	// c++ 11
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;

Peer peer;
vector<std::thread>	peerDis;
int*  HaveLock;
std::mutex mtx;
std::mutex mtx2;
std::condition_variable cv;
bool ready = false;
int wait_left = 0;

void registerAndListen ( ServerSocket* &uploaderSock, 
	ClientSocket* &sockToTracker )
{
	// Register on Tracker...
	// First create a listening (server) socket..
	// Randomly select a port number (from OS)

	uploaderSock = new ServerSocket(rand()%2000+8100);
	int alloPort = uploaderSock->getListeningPort();
	std::cout << "Listening to other peers at Port "
		<< alloPort << ".\n";

	// The client socket connected to tracker
	sockToTracker = new ClientSocket( TRACKER_IP, TRACKER_PORT );
	char* sendEntry = new char[80] {};

	peer.trackerRegister( sendEntry, alloPort );
	if (! sockToTracker->send( sendEntry, 
		TransCHAR( sendEntry ) ) )
	{
		delete sockToTracker;
		ErrorSend e;
		throw e;
	}
	delete[] sendEntry; 

	char* trackerRespond;
	unsigned int recvLen = 
		sockToTracker->recv(  trackerRespond );
	if (recvLen == 0){
		delete sockToTracker;
		NoResponseWhileShould e;
		throw e;
	}
	std::cout << "Successfully registering on Tracker.\n";	
	delete[] trackerRespond;
	delete sockToTracker;

	// sockToTracker deleted outside since 
	// downloader still uses this sock for peerlist Req.
	// uploaderSock is always on.
	return;
}

void emptyThread( void )
{
//	cout << "This is an empty thread.." << endl;
	return;
}

void run()
{
	std::unique_lock<std::mutex> lck(mtx);
	mtx2.lock();
	ready = true;
	cv.notify_all();
	mtx2.unlock();
}

void peerDistribution( ClientSocket* sockToPeer, int age, int index )
// age 0 old(existing) peer in network, age 1 new (incoming) peer
{
	try
	{
		// Each thread maintains a array alreadySendHave
		// If aSH[i] == 0 and HL[i] == 1, then send HAVE<i>
		// After that, set aSH[i] = 1
		unsigned int numPieces = peer.getBitfieldLength();
		int* alreadySendHave = new int[numPieces] {};

		if ( age == 1 )
		// If I am the new peer..
		// Send handshake to peer.
		{
			char* handShake = new char[20] {};
			peer.handshakePeers( handShake );
			if (! sockToPeer->send( handShake, 
				TransCHAR( handShake ) ) )
			{
				ErrorSend e;
				throw e;
			}
			// Then receive bitfield from old peer..	
			char* bitfield;
			unsigned int recvLen = 
				sockToPeer -> recv(  bitfield );
			if (recvLen == 0){
				NoResponseWhileShould e;
				throw e;
			}
			
			peer.onReceiveBitfield( bitfield, index );
			delete[] handShake;
			delete[] bitfield;
			wait_left--;
			if ( wait_left == 0 ) {
				mtx2.unlock();
			}	

			std::unique_lock<std::mutex> lck(mtx);
			while (!ready) cv.wait(lck);	
		}
		else //age == 0
		// Receive handshake and send bitfield
		{
			char* handShake;
			unsigned int recvLen = 
				sockToPeer -> recv(  handShake );
			if (recvLen == 0){
				NoResponseWhileShould e;
				throw e;
			}
			char*	bitfield = new char[BITFIELD_LEN] {};
			peer.onReceiveHandshake( handShake, bitfield );
			for (int i = 0; i < peer.getBitfieldLength(); i++)
			{
				if ( bitfield[17 + i] == uint8_t (1) )
				{
					alreadySendHave[i] = 1;
				}
			}
			if (! sockToPeer->send( bitfield, 
				TransCHAR( bitfield ) ) )
			{
				ErrorSend e;
				throw e;
			}
			delete[] handShake;
			delete[] bitfield;
		}

		// Bitfield get. start peer distribution..

		// True after the other peer has finished transmission
		// And send me a FINISH signal
		bool recvFinish = false;
		bool uploader = peer.checkFinish();

		// True atfer I have finished recving.
		bool checkFinish = uploader;
	
		// the thread survives till both me and the other one
		// have owned the complete file.
		while ( ( !uploader && !checkFinish ) || !recvFinish )
		{
			// Need: detect chosen seems no use
			// Need detectWant()
			// Return piece pieceWant from peer index
			// Or return -1 (No need now from peer index)
			// ANS: done.

			if ( !uploader && !checkFinish ){

				// If other threads have a new piece
				// send HAVE to my peer.
				for (int i = 0; i < numPieces; i++ ){
					if ( alreadySendHave[i] == 0 && HaveLock[i] == 1 )
					{
						char* have = new char[20] {};
						peer.sendBroadcastToPeers( i, have );
						if (! sockToPeer->send( have, 
							TransCHAR( have ) ) )
						{
							ErrorSend e;
							throw e;
						}
						alreadySendHave[i] = 1;
						delete[] have;
					}
				}

				// Probably send Req to peer
				int pieceWant = peer.detectWant( index );
				if ( pieceWant >= 0 ){
					char* blockReq = new char[20] {};
					peer.requestReceiveBlock( pieceWant , blockReq );	
					if (! sockToPeer->send( blockReq, 
						TransCHAR( blockReq ) ) )
					{
						ErrorSend e;
						throw e;
					}
					delete[] blockReq;
				}	

				// If finish, send finish to my peer
				checkFinish = peer.checkFinish();
				if ( checkFinish ){
					char* finish = new char[20] {};
					peer.sendFinish( finish );
					if (! sockToPeer->send( finish, 
						TransCHAR( finish ) ) )
					{
						ErrorSend e;
						throw e;
					}
					delete[] finish;					
				}				
			}

			// During each polling cycle,
			// Once recved sth from peer, continue to check
			// if there is anything to send.
			// Otherwise, wait for 3 sec if nothing from peer

			char* packet;
			unsigned int recvLen = 
				sockToPeer -> recv(  packet );
			if (recvLen == 0) continue; // to next cycle
			else{
				// Recved something..
				MessageID type = (MessageID)packet[4];

				if ( type ==  REQ ){

					// This is a Req packet.
					// Need: peer.cpp changed our protocal.
					// Blocknum should be in the header field
					// blocklength and (offset) should also be there.

					// In onReceiveFragment, blocknum will not 
					// be provided as argument -- should be read
					// from header.
					// Payload is purely file content.

					// On receive a block, the peer compare it 
					// with its own .torrent file, if no diff, 
					// store the block (update bitfield) and 
					// return the recved blocknum for main. 
					// Otherwise, return -1.
					// Need: add return value to oRF().

					// In main, wrong block recved will simply be dropped.
					// and right block will trigger HAVE boardcast.

					char* reply = new char[BLOCK_MAX_SIZE] {};
					peer.onReceiveRequestForBlock( packet, reply );

					if (! sockToPeer->send( reply, 
						TransCHAR( reply ) ) )
					{
						ErrorSend e;
						throw e;
					}
					delete[] reply;
				}
				else if ( type == REPLY )
				{
					// See above for modification for oRF.
					int numBlock;
					numBlock  = peer.onReceiveFragment( packet );
					if ( numBlock >= 0 ){
						HaveLock[numBlock] = 1;
					}
				}	
				else if ( type == HAVE )
				{
					peer.onReceiveBroadcastFromPeers( packet, index );
				}
				else if ( type == FINISH )
				{
					recvFinish = true;				
				}	
			}
			delete[] packet;
		}
		delete[] alreadySendHave;
		delete sockToPeer;
		return;
	}
	catch (ErrorSend){
		std::cout << index << " :Error send.\n";
		delete sockToPeer;
		return;		
	}
	catch (SocketError){
		std::cout << index << " SIGNAL from peer.. Thread terminated..\n";
		delete sockToPeer;
		return;
		// Just terminate this thread
	}
	catch ( NoResponseWhileShould ){
		std::cout << index << " :No response while should.\n";
		// Just terminate the thread.
		delete sockToPeer;
		return;
	}
}

void uploadControl( ServerSocket* uploaderSock )
{
	// If someone connect to uploaderSock...
	while (1){
		ClientSocket* inComingPeer = uploaderSock -> accept();

		struct peer_info thispeer = inComingPeer->getPeerInfo();
		std::cout << "Connected to Peer " << thispeer.ip << " at Port " << thispeer.port << ".\n";
		peer.updatePeerInfo( thispeer ); 

		//Then start a new thread for peer distribution
		int index = peerDis.size();
		peerDis.push_back( std::thread ( 
			peerDistribution, inComingPeer, 0, index ) );
	}
}

int main()
{
	try
	{
		srand((unsigned int)time(NULL));
	//  d or c + filename
		std::cout << "Please input command...\n";
		string command;
		std::cin >> command;
		if (command != "c" && command != "d") {
			InvalidCommand e;
			throw e;
		}
		if ( command == "c" ){
			// Input filename , create, upload .torrent file
			char* pathname = new char[80] {};
			std::cin >> pathname;
			char* torrentName = new char[80] {};
			peer.createTorrent( pathname, torrentName );
			// try uploading.. The server may reject due to duplicate name.
			// The peer may disgard due to different sha1 code.
			// Send ack to server if sha1 codes match.
			char* TorrentFile = new char[ TORRENT_MAX_SIZE ] {};
			peer.uploadTorrent( torrentName, TorrentFile );
			ClientSocket* sockToServer = 
				new ClientSocket( SERVER_IP, SERVER_PORT );
			if (! sockToServer->send( TorrentFile, TransCHAR(TorrentFile) ) )
			{
				delete sockToServer;
				ErrorSend e;
				throw e;
			}
			delete[] pathname;
			delete[] TorrentFile;

			char* serverRespond;
			unsigned int recvLen = 
				sockToServer->recv(  serverRespond );
			if (recvLen == 0){
				delete sockToServer;
				NoResponseWhileShould e;
				throw e;
			}

			char* respondToServer = new char[20] {};
			bool recvSuccess;
			recvSuccess =  peer.onReceiveServerRespond( 
				serverRespond, respondToServer );
			if (! sockToServer->send( respondToServer, 
				TransCHAR(respondToServer) ) )
			{
				delete sockToServer;
				ErrorSend e;
				throw e;
			}
			delete[] serverRespond;
			delete[] respondToServer;
			delete sockToServer;
			if (! recvSuccess ){
				std::cout << "Uploading Failed..Try again..\n";
				return EXIT_FAILURE;
			}
			std::cout << "Successfully uploaded " 
				<< torrentName << " to server.\n";

			delete[] torrentName;

			// register on Tracker and start listening..
			ServerSocket* uploaderSock;
			ClientSocket* sockToTracker;
			registerAndListen( uploaderSock, sockToTracker );

			// Wait for peers' connection in a new thread.
			std::thread upC (uploadControl, uploaderSock);	

			// wait for all the others to finish. 
			// Actually stall here forever..
			upC.join();

		}
		else{
			// Command equals "d"
			// download the list first

			char* listReq = new char[20] {};
			peer.requestDownloadTorrent( listReq );

			// Connect to Server and send request
			ClientSocket* sockToServer = new ClientSocket( SERVER_IP, SERVER_PORT );
			if (! sockToServer->send( listReq, 
				TransCHAR(listReq) ) )
			{
				delete sockToServer;
				ErrorSend e;
				throw e;
			}	
			// get response from server (a list)		
			char* serverRespond;
			unsigned int recvLen = 
				sockToServer->recv(  serverRespond );
			if (recvLen == 0){
				delete sockToServer;
				NoResponseWhileShould e;
				throw e;
			}
			peer.onReceiveReplyDownloadTorrent( 
				serverRespond );
			delete[] listReq;
			delete[] serverRespond;
			serverRespond = NULL;
			delete sockToServer;

			cout << "Please enter the filename to download:\n";
			char* filename = new char[50] {};
			cin >> filename;

			char* downloadReq = new char[80] {};
			peer.chooseDownloadTorrent(filename, downloadReq);
			sockToServer = 
				new ClientSocket( SERVER_IP, SERVER_PORT );
			if (! sockToServer->send( downloadReq, 
				TransCHAR( downloadReq ) ) )
			{
				delete sockToServer;
				ErrorSend e;
				throw e;
			}
			delete[] filename;
			delete[] downloadReq;
			// get response from server (Torrent file)

			recvLen = 
				sockToServer->recv(  serverRespond );
			if (recvLen == 0){
				delete sockToServer;
				NoResponseWhileShould e;
				throw e;
			}
			peer.onReceiveDownloadTorrent( 
				serverRespond );
			delete[] serverRespond;
			delete sockToServer;

			// register on Tracker and start listening..
			ServerSocket* uploaderSock;
			ClientSocket* sockToTracker;
			registerAndListen( uploaderSock, sockToTracker );

			// request peer list from tracker
			sockToTracker = new ClientSocket( TRACKER_IP, TRACKER_PORT );
			char* peerListReq = new char[100] {};
			peer.requestPeerList( peerListReq );
			if (! sockToTracker ->send( peerListReq, 
				TransCHAR( peerListReq ) ) )
			{
				delete sockToTracker;
				ErrorSend e;
				throw e;
			}
			char* trackerRespond;	
			recvLen = sockToTracker->recv(  trackerRespond );
			if (recvLen == 0){
				delete sockToTracker;
				NoResponseWhileShould e;
				throw e;
			}
			peer.onReceivePeerlist( trackerRespond );	
			delete[] peerListReq;
			delete[] trackerRespond;
			delete sockToTracker;
			// Wait for peers' connection in a new thread.
			std::thread upC2 (uploadControl, uploaderSock);
			
			// Initialize all the HaveLocks for each block to 0.
			// This is a shared variable between all peerDis.
			// If one thread peerDis change HaveLock[i] to 1,
			// All the threads should send HAVE<i>.
			unsigned int numPieces = peer.getBitfieldLength();
			HaveLock = new int[numPieces] {};
			// At the same time, start downloading..
			// First, start a new thread for each peer recved 
			// from peerList (excluding itself)

			int peerNum = peer.getNumPeers();

			mtx2.lock();
			for ( int i = 0; i < peerNum; i++ ){
				struct peer_info pi_now = peer.getPeerInfo( i );
				try {
					ClientSocket* sockToPeer = 
					new ClientSocket( pi_now.ip, pi_now.port );
					std::cout << "Connected to Peer " << pi_now.ip 
						<< " at Port " << pi_now.port << ".\n"; 
					wait_left ++;
					peerDis.push_back(std::thread(peerDistribution, sockToPeer, 1, i));
				}
				catch (ConnectionFailure) {
				// If connection fail, just push back an empty thread.
					std::cout << "Unable to connect peer " << i << ".\n";
					peer.pushEmptyBitfield( i );
					peerDis.push_back( std::thread(emptyThread) );
				}
			}
	
			run();
			// After finishing downloading from each peer 
			// ie (each peer) terminates after seeing bitfield full
			for ( int j = 0; j < peerDis.size(); j++ ){
				peerDis[j].join();
			}
			delete[] HaveLock;
			upC2.join();
			// wait for all the others to finish. 
			// Actually stall here forever..
			
		}
	}
	catch( InvalidCommand )
	{
		std::cout << "Invalid Command.\n";
		return EXIT_FAILURE;
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
	catch ( NoResponseWhileShould ){
		std::cout << "No response while should.\n";
		return EXIT_FAILURE;
	}
	catch ( SocketError )
	{
		std::cout << "Socket Error.\n";
		return EXIT_FAILURE;
	}
	catch ( ConnectionFailure )
	{
		std::cout << "Connection Failure to server/tracker.\n";
		return EXIT_FAILURE;
	}
}