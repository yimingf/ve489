// server.cpp

#include <cstring>
#include <fstream>
#include "Server.h"
#include "Packet.h"

bool Server::hasName(char* name) {
	// detect if the name is inside the vector.
	// if so, return true. otherwise return false.
	for (int i = 0; i < torrent_list.size(); i++) {
		if (torrent_list[i].name == name) {
			return true;
		}
	}
	return false;
}

bool Server::createTorrent (char* stream) {
	Packet 				pkt(stream);
	char* 		str;
	str = pkt.getPayload();
	char* 		foo;
	foo = (char*)memchr(str, '\n', strlen(str));
	foo++;
	char		name[foo-str];
	memmove( name, str, foo-str );
	name[foo-str-1] = '\0';
	if (!hasName(name)) {
		// no torrent name found.
		// will create a new torrent file.
		ofstream		file(name);
		file.write(foo, pkt.getPlength() - (foo-str));
		file.close();
		// write the torrent file.

		torrent_t 		torrent;
		for (int i = 0; i < (foo-str); i++) {

			torrent.name[i]    = name[i];
		}
		torrent.name[foo - str] = '\0';

		unsigned char* temp = new unsigned char[SHA_DIGEST_LENGTH] {};
		SHA1((unsigned char*)foo, pkt.getPlength() - (foo-str), temp);
		for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
			sprintf((char*)&(torrent.hashcode[i*2]), "%02x", temp[i]);
		}
		delete[] temp;

		torrent_list.push_back(torrent);
		// insert the map.
		return true;
	}
	else {
		return false;
	}
}

void getTorrentContent (char* foo, char* name) {
	// write the torrent file content to the 
	sprintf (foo, "%s\n", name);
	// name of the torrent file.
	ifstream 		file(name, std::ifstream::binary);
	file.seekg(0, file.end);
	int 			length = file.tellg();
	file.seekg(0, file.beg);
	char 			bar[length + 1];
	file.read(bar, length);
	bar[length] = '\0';
	file.close();
	// the content.
	sprintf (foo, "%s%s", foo, bar);
}

int getTorrentLength (char* name) {
	// get the length of the payload.
	int 			foo = 0;
	// the length of the name.
	foo += (strlen(name) + 1);
	ifstream 		file(name, std::ifstream::binary);
	file.seekg(0, file.end);
	foo += file.tellg();
	// the length of the content.
	file.close();
	return foo;
} 

void Server::onReceiveTorrent (char* stream, char* foo) {
	if (createTorrent(stream)) {
	// create the torrent if no collision.
		Packet 			ack(ACK, torrent_list.back().hashcode, SHA_DIGEST_LENGTH*2);
		ack.generateChunk(foo);
		// send the ack w/the sha-1 code.
	}
	else {
		Packet 			nak(NAK);
		nak.generateChunk(foo);
	}
}

void Server::onRequestTorrentList (char* stream, char* foo) {
	Packet pkt(stream);
	// decrypt the packet.
	char 		bar[1000];
	bar[0] = '\0';
	for (int i = 0; i < torrent_list.size(); i++) {
		sprintf (bar, "%s%s\n", bar, torrent_list[i].name);
	}
	Packet t_list(torLIST, bar, strlen(bar));
	t_list.generateChunk(foo);
}

void Server::onRequestTorrentSpecified (char* stream, char* foo) {
	Packet 		pkt(stream);
	// decrypt the packet.
	char* 		bar;
	bar = pkt.getPayload();
	char 		baz[pkt.getPlength()+1];
	memmove(baz, bar, pkt.getPlength());
	baz[pkt.getPlength()] = '\0';
	printf("[server] request torrent name: %s\n", baz);
	if (!hasName(baz)) {
		char 	zaq[TORRENT_MAX_SIZE];
		getTorrentContent(zaq, baz);
		Packet 			torrent(torFILE, zaq, getTorrentLength(baz));
		// need calculation for length.
		torrent.generateChunk(foo);
	}
	else {
		Packet 			nak(NAK);
		nak.generateChunk(foo);
	}
}

void Server::deleteLatestFile() {
	torrent_list.pop_back();
}