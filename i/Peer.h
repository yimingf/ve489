#ifndef __PEER_H__
#define __PEER_H__

#include <string>
#include <vector>

#include "fragment.h"
#include "File.h"
#include "Packet.h"
#include <openssl/sha.h>

using namespace std;

class Peer {
	// every peer is responsible for one single file.
protected:

	vector<peer_info> 		peers;
	vector<uint8_t*>		bitfields;
	File*				originalFile;
	char				hashcode[SHA_DIGEST_LENGTH * 2];

	void 				writeToFileClass(char* name);
	// write to the originalFile from the torrent file.

public:

	Peer() {}
	~Peer() 
	{
		for (int i = 0; i < bitfields.size(); i++){
			delete[] bitfields[i];
		}
		delete originalFile;
	}
	// constructor and the destructor.

	int					getNumPeers();
	unsigned int 		getBitfieldLength();
	struct peer_info 	getPeerInfo(int index);
	// getters.

	void		createTorrent(char* filename, char* pkt);
	// create a torrent file for corresponding file.
	// if successfully created, return 1.
	// otherwise, return 0.
	void		 		uploadTorrent(char* filename, char* pkt);
	// upload the torrent to the server.
	// if success, create a file class.
	// otherwise, will return nak.
	bool 				onReceiveServerRespond(char* stream, char* pkt);
	// return true is received ack.
	// otherwise return false and terminate the program.
	// would send a packet.
	void		 		trackerRegister(char* pkt, unsigned short port);
	// register to the tracker.
	// if success, return ack.
	void		 		requestDownloadTorrent(char* pkt);
	// request the server for download.
	void 				onReceiveReplyDownloadTorrent(char* stream);
	// if success, receive a list and display onto the terminal.
	// otherwise, display the nak.
	void				chooseDownloadTorrent(char* filename, char* pkt);
	// send the name of chosen torrent to the server.
	// if success, will start downloading.
	void 				onReceiveDownloadTorrent(char* stream);
	// if filename match w/svr, will start peer dist.
	// otherwise, return nak.
	// if peer dist starts, the packet will return
	// necessary info for the new file.
	void		 		requestPeerList(char* pkt);
	// rQ the peer list from the tracker.
	void 				onReceivePeerlist (char* stream);
	// if success, will return a list of peers.
	// saved in the class "peers".
	void		handshakePeers(char* pkt);
	// create a packet to send to all potential peers.
	// would be sent by different sockets.
	void 		onReceiveHandshake(char* stream, char* pkt);
	// if everything ok, send the bitfield back.
	void 		onReceiveBitfield(char* stream, unsigned int peerNumber);
	// save the bitfield into the 
	int 		detectWant(int index);
	// for those chosen peers, detect which *certain* fragment
	// is most wanted.
	// for those most wanted, mark the fragment number.
	void 		requestReceiveBlock(unsigned int blockNumber, char* pkt);
	// select the wanted fragment and send request to some peer.
	void 		onReceiveRequestForBlock(char* stream, char* pktFoo);
	// if the peer have the block, send it.
	int 		onReceiveFragment(char* stream);
	// if transfer success, the fragment saved in the big temp file.
	void 		sendBroadcastToPeers(int blockNumber, char* pkt);
	// once the block finishes transferring, broadcast to all peers.
	// will send a packet w/broadcast format.
	void 		onReceiveBroadcastFromPeers(char* stream, int index);
	// update bitfields from others.
	void 		updatePeerInfo(struct peer_info);
	// pushback peer info and set bitfield to 0.
	bool 		checkFinish();
	// if finished, return true.
	void 		sendFinish(char* pkt);
	// send an empty packet w/finish type.
	void 		pushEmptyBitfield(int index); 
};

#endif
