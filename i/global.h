// global.h

#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <openssl/sha.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "math.h"

//#define MAX_BUFFER_SIZE 1200
#define TIME_OUT_SEC 		5
#define TIME_OUT_MS	 	0
#define TORRENT_MAX_SIZE 	131072
#define BITFIELD_LEN 		1000
#define BLOCK_MAX_SIZE 		2097200 //After appending header 	
#define BLOCKSIZE 		2097152 //1048576 2097152

struct peer_info {
	char 	ip[20];
	unsigned short port;
};

struct torrent_info {
	struct peer_info 	peer;
	char 				hashcode[SHA_DIGEST_LENGTH * 2];
};

enum MessageID {
	torFILE, ACK, NAK, HANDSHAKE, hREPLY,
	REQ, REPLY, HAVE, torLIST,
	FINISH, REQFile, DEFAULT
};

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8800
#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 8801

unsigned int TransCHAR(char c[]);
void TransINT(unsigned int i, char c[]);

class EmptyPacket{
};

class EmptyTracker{
};

class InvalidCommand{};

class ErrorSend{};

class SocketError{};

class NoResponseWhileShould{};

class ConnectionFailure{};

class ExistingTorrentName{};

#endif
