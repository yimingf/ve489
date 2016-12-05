// Peer.cpp

#include <string>
#include <vector>

#include "Peer.h"

struct peer_info getPeerFromChar (char* str) {
	peer_info peer;
	char* 	foo;
	char 	ip[20];
	char 	port[10];
	foo = (char*) memchr(str, ':', strlen(str));
	// get the colon for division.
	memmove(ip, str, (foo-str));
	ip[foo-str] = '\0';
	memmove(port, foo+1, (str+strlen(str)-foo-1));
	port[(str+strlen(str)-foo-1)] = '\0';
	// move them to the char.
	strcpy(peer.ip, ip);
	peer.port = (unsigned short) atoi(port);
	return peer;
}

int Peer::getNumPeers() {
	// get number of peers.
	return peers.size();
}

struct peer_info Peer::getPeerInfo(int index) {
	return peers[index];
}

void Peer::createTorrent (char* filename, char* bar) {
	char		foo[TORRENT_MAX_SIZE];
	// the torrent content.
	sprintf (foo, "%s", filename);
	sprintf (bar, "%s.torrent", foo);
	// get the name of the torrent (bar).
	ofstream 	torrentFile(bar);
	originalFile = new File(filename);
	// create a file object.
	sprintf (foo, "%s\n%d\n%d\n", foo, originalFile->getLength(), originalFile->getNumPieces());
	// write the length and the number of pieces.
	for (int i = 0; i < originalFile->getNumPieces(); i++) {
		sprintf (foo, "%s%s\n", foo, originalFile->readFragment(i).hashcode);
	}
	// write the hashcode of the files.
	// now content created.
	unsigned char* temp = new unsigned char[SHA_DIGEST_LENGTH] {};
	SHA1( (unsigned char*)foo, strlen( foo ), temp );
	for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(hashcode[i*2]), "%02x", temp[i]);
	}
	delete[] temp;
	// create the hashcode of the torrent.
	torrentFile.write(foo, strlen(foo));
	// write to the torrent.
	torrentFile.close();
}

void Peer::uploadTorrent (char* filename, char* baz) {
	ifstream 		file(filename);
	file.seekg(0, file.end);
	int 			length = file.tellg();
	file.seekg(0, file.beg);
	char* 	foo = new char[length + 1] {};
	// get the length of the torrent file.
	file.read(foo, length);
	file.close();
	char*	bar = new char[length + 100] {};
	sprintf (bar, "%s\n%s", filename, foo);
	delete[]		foo;
	// create the payload.
	length += (strlen(filename)+1);
	Packet 			pkt(torFILE, bar, strlen( bar ) );
	pkt.generateChunk(baz);
	delete[]		bar;
}

bool Peer::onReceiveServerRespond (char* stream, char* foo) {
	Packet 			pkt(stream);
	char* 			bar;
	char 			baz[SHA_DIGEST_LENGTH * 2];
	bar = pkt.getPayload();
	memmove(baz, bar, SHA_DIGEST_LENGTH * 2);
	if (! strcmp( baz, hashcode ) ) {
		Packet 	ack(ACK);
		ack.generateChunk(foo);
		return true;
	} else {
		Packet 	nak(NAK);
		nak.generateChunk(foo);
		return false;
	}
}

void Peer::trackerRegister(char* foo, unsigned short port) {
	char	bar[256];
	sprintf (bar, "%d\n%s\n", port, hashcode);
	// get the port and hashcode.
	Packet 			pkt(HANDSHAKE, bar, strlen(bar));
	pkt.generateChunk(foo);
}

void Peer::requestDownloadTorrent(char* foo) {
	Packet 			pkt(REQ);
	pkt.generateChunk(foo);
	// send the message.
}

void Peer::onReceiveReplyDownloadTorrent (char* stream) {
	Packet 			pkt(stream);
	char* 	bar;
	bar = pkt.getPayload();
	// get the torrent list.
	bar[pkt.getPlength()] = '\0';
	if (bar) {
		printf ("%s", bar);
		// display onto the terminal.
	}
	else {
		cout << "error occured when receiving torrent list" << endl;
	}
}

void Peer::chooseDownloadTorrent (char* filename, char* foo) {
	Packet 			pkt(REQFile, filename, strlen(filename));
	pkt.generateChunk(foo);
}

void Peer::onReceiveDownloadTorrent (char* stream) {
	Packet 			pkt(stream);
	if (pkt.getType() == NAK) {
		cout << "invalid torrent name!" << endl;
		return;
	}
	// somehow reused the code from server.cpp:
	// "createTorrent".
	char* 	str;
	str = pkt.getPayload();
	char*	foo;
	foo = (char*)memchr(str, '\n', strlen(str));
	foo++;
	char 			name[foo-str];
	memmove( name, str, foo - str);
	name[foo-str-1] = '\0';
	// get the torrent file name.
	ofstream 		file(name);
	file.write(foo, pkt.getPlength() - (foo - str));
	file.close();
	writeToFileClass(name);
	// initialize hashcode since we are going to register with it
	unsigned char* temp = new unsigned char[SHA_DIGEST_LENGTH] {};
	SHA1( (unsigned char*)foo, pkt.getPlength() - (foo - str), temp );
	for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(hashcode[i*2]), "%02x", temp[i]);
	}
	delete[] temp;
	// write the torrent file.
}

void Peer::writeToFileClass (char* name) {
	ifstream 	torrentFile(name);
	string 		foo, bar;// name and the length
	getline(torrentFile, foo);
	char* 	fname = new char[64] {};
	strcpy( fname, foo.c_str() );
	getline(torrentFile, bar);
	char 		tmp[20];
	strcpy( tmp, bar.c_str() );
	originalFile = new File(fname, atoi(tmp) );
	delete[] fname;
	// create the file.
}

void Peer::requestPeerList (char* foo) {
	Packet 			pkt(REQ, hashcode, SHA_DIGEST_LENGTH * 2);
	pkt.generateChunk(foo);
}

void Peer::onReceivePeerlist (char* stream) {
	Packet 			pkt(stream);
	char* 			str;
	str = pkt.getPayload();
	str[pkt.getPlength()] = '\0';
	if (!str) {
		cout << "error occured on receiving peer list" << endl;
		return;
	}
	char* 		foo;
	char* 		bar;
	int 		length = pkt.getPlength();
	bar = str;
	while (bar < (str+length)) {
		foo = (char*) memchr(bar, '\n', str + strlen(str) - bar);
		foo++;
		char peerStr[foo-bar];
		memmove(peerStr, bar, (foo-bar));
		peerStr[foo-bar-1] = '\0';
		// generate the peer string.
		struct peer_info peer = getPeerFromChar(peerStr);
		// generate the struct from string.
		peers.push_back(peer);
		uint8_t* peer_bit = NULL;
		bitfields.push_back( peer_bit );
		// push it into the vector.
		bar = foo;
	}
}

void Peer::handshakePeers(char* foo) {
	Packet 		pkt(HANDSHAKE);
	pkt.generateChunk(foo);
}

void Peer::onReceiveHandshake(char* stream, char* foo) {
	Packet 				pkt(stream);
	if (pkt.getType() != HANDSHAKE) {
		cout << "error occured when receiving handshake" << endl;
		return;
	}
	uint8_t 		bitfield[BITFIELD_LEN];
	sprintf( (char*)bitfield, "%s", originalFile->getBitfield() );
	Packet 		pktFoo( hREPLY, (char*)bitfield, originalFile->getNumPieces() );
	// reply w/bitfield.
	pktFoo.generateChunk(foo);
}

void Peer::onReceiveBitfield(char* stream, unsigned int peerNumber) {
	Packet 			pkt(stream);
	uint8_t* peer_bit = new uint8_t[originalFile->getNumPieces()] {};
	char* tmp = pkt.getPayload();
	memmove( (char *) peer_bit, tmp,  originalFile->getNumPieces() );
	bitfields[peerNumber] = peer_bit;
	// save the bitfield into the vector.
}

int Peer::detectWant(int index) {
	uint8_t bitfield[originalFile->getNumPieces()];
	memmove(bitfield, originalFile->getBitfield(), originalFile->getNumPieces());
	// get the self bitfield.
	int 		healthMin = BLOCKSIZE;
	int 		numMin = -1;
	int 		pieces = originalFile->getNumPieces();
	// get the pieces and initialize the minimum health.
	for (int i = 0; i < pieces; i++) {
		if (bitfield[i] != (uint8_t)(-1) ){
			continue;
		} else if (bitfields[index][i] == (uint8_t)(-1) ){
			continue;
		} else {
			int health = 0;
			for (int j = 0; j < bitfields.size(); j++) {
				if (bitfields[j][i] == 1) {
					health++;
				}
				// calculate the health in the whole peers.
			}
			if (health < healthMin) {
				numMin = i;
				healthMin = health;
			} else if (health == healthMin) {
		//		srand((unsigned int)time(NULL));
				int p = rand() % originalFile->getNumPieces();
				if ( p < 4 ) numMin = i;
				// create disturbance.
			}
		}
	}
	if (numMin != -1) {
		originalFile->setBitfield(numMin, 2);
	}
	// set the bitfield value to 1.
	return numMin;
}

void Peer::requestReceiveBlock(unsigned int blockNumber, char* foo) {
	unsigned int request[3];
	request[0] = blockNumber;
	Packet 			pkt(REQ, request);
	pkt.generateChunk(foo);
}

void Peer::onReceiveRequestForBlock(char* stream, char* foo) {
	Packet 			pkt(stream);
	unsigned int 	request[3];
	pkt.getRequest( request );
	unsigned int 	blockNumber = request[0];
	// get the block number.
	if ( originalFile->getBitfield()[blockNumber] == -1) {
		cout << "error occured when receiving requested block number: requested block not exist" << endl;
		return;
	}
	struct fragment  frag = originalFile->readFragment(blockNumber);
	// get the fragment of the file.
	Packet 			pktFoo(REPLY, request, frag.bitstream, BLOCKSIZE);
	cout << "[peer_up] send fragment number " << blockNumber << endl;
	pktFoo.generateChunk(foo);
	// send the bitstream.
}

int Peer::onReceiveFragment(char* stream) {
	Packet 			pkt(stream);
	char* 	foo;
	foo = pkt.getPayload();
	fragment 		frag;
	unsigned int 	request[3];
	pkt.getRequest( request );
	frag.numPieceSelf = request[0];
	cout << "[peer_down] received fragment number: " << request[0] << endl;
	memmove(frag.bitstream, foo, pkt.getPlength());
	// create a fragmnt.
	originalFile->saveFragment(frag);
	// save the fragment into the file.
	return frag.numPieceSelf;
	// need further implementations.
}

void Peer::sendBroadcastToPeers (int blockNumber, char* foo) {
	char 	str[8];
	sprintf (str, "%d\n", blockNumber);
	Packet 	pkt(HAVE, str, strlen(str));
	pkt.generateChunk(foo);
}

void Peer::onReceiveBroadcastFromPeers(char* stream, int index) {
	Packet 	pkt(stream);
	if (pkt.getType() != HAVE) {
		cout << "error occured when receiving broadcast: packet type not match" << endl;
		return;
	}
	char* 	foo;
	foo = pkt.getPayload();
	int 	blockNumber = atoi(foo);
	bitfields[index][blockNumber] = 1;
}

void Peer::updatePeerInfo ( struct peer_info peer ) {
	uint8_t* peer_bit = new uint8_t[originalFile->getNumPieces()] {};
	for (int i = 0; i < originalFile->getNumPieces(); i++) {
		peer_bit[i] = -1;
	}
	peers.push_back(peer);
	bitfields.push_back( peer_bit );
}

unsigned int Peer::getBitfieldLength() {
	return originalFile->getNumPieces();
}

bool Peer::checkFinish() {
	return originalFile->isComplete();
}

void Peer::sendFinish(char* foo) {
	Packet 		pkt(FINISH);
	pkt.generateChunk(foo);
}

void Peer::pushEmptyBitfield(int index) {
	uint8_t* peer_bit = new uint8_t[originalFile->getNumPieces()] {};
	for (int i = 0; i < originalFile->getNumPieces(); i++) {
		peer_bit[i] = -1;
	}
	bitfields[index] = peer_bit;
}