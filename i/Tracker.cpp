#include "Tracker.h"

Tracker::Tracker(){}
Tracker::~Tracker(){}

void Tracker::onPeerRegister(char* stream,
	ClientSocket* csocket, char* peerlistpkt) {
	//HANDSHAKE
	Packet 				pkt(stream);
	
	if (pkt.getType()==HANDSHAKE){
		torrent_info 	tor;
		char* 			tmp;//port \n hashcode
		unsigned int 	length;
		tmp = pkt.getPayload();
		length = (char*)memchr(tmp,'\n',strlen(tmp))-tmp;
		char* 	portShort = new char[length + 1] {};
		for (int i = 0; i < (int)length; i++) {
			portShort[i] = tmp[i];
		}
		tor.peer.port=(unsigned short)atoi(portShort);
		delete[] portShort;
	
		char* 			foo = (char*)memchr(tmp,'\n',strlen(tmp)) + 1;
		memmove( tor.hashcode, foo, SHA_DIGEST_LENGTH * 2 );
		strcpy( tor.peer.ip, csocket->getPeerInfo().ip);
		peer_list.push_back(tor);
		//return ACK
		printf("[tracker] handshake from %s:%d\nhashcode: %s\n", tor.peer.ip, tor.peer.port, tor.hashcode);
		Packet			ack(ACK);
		ack.generateChunk(peerlistpkt);
	} else if (pkt.getType()==REQ){
		char*		tmp=new char[2000] {};
		char* 		Req_hashcode;
		Req_hashcode = pkt.getPayload();
		for (int i=0;i<peer_list.size() - 1;i++){
			char 	bar[SHA_DIGEST_LENGTH * 2 + 1];
			char 	baz[SHA_DIGEST_LENGTH * 2 + 1];
			memmove(bar, peer_list.at(i).hashcode, SHA_DIGEST_LENGTH * 2 + 1);
			memmove(baz, Req_hashcode, SHA_DIGEST_LENGTH * 2 + 1);
			bar[SHA_DIGEST_LENGTH * 2] = '\0';
			baz[SHA_DIGEST_LENGTH * 2] = '\0';
			if (! strcmp( bar, baz ) ) {
				sprintf(tmp,"%s%s:%d\n", tmp, peer_list.at(i).peer.ip,peer_list.at(i).peer.port);

			}
		}
		//return list
		printf("[tracker] peer list: %s\n", tmp);
		Packet			reply(REPLY, tmp, strlen(tmp));
		reply.generateChunk(peerlistpkt);
		delete[] tmp;
	} else {
		EmptyTracker empty;
		throw empty;
	}
}

