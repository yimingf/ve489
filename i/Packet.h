#ifndef __PACKET_H__
#define __PACKET_H__

#include "global.h"
#include <iostream>
#include <math.h>
using namespace std;

class Packet {

protected:

	unsigned int 	length;
	// indicate the length of the Packet.
	MessageID		type;
	// the type message ID
	char* 			payload;
	// here "payload" could refer to various types of data.
	char 			request[3][4];
	// here the array coordinates w/the situation when the type is "rQ"/"rP".
	void setrequest();

public:

	Packet();
	Packet(char* chunk);
	// decrypt from bitstream.
	Packet(MessageID t);
	Packet(MessageID t, unsigned int* r);
	Packet(MessageID t, char* p, unsigned int plength);
	Packet(MessageID t, unsigned int* r, char* p, unsigned int plength);
	~Packet();
	// constructors (and destructor).

	unsigned int 	getLength();
	MessageID 		getType();
	char*			getPayload();
	void 			getRequest(unsigned int r[]);
	// getters.
	unsigned int 	getPlength();//payload length
	void			generateChunk(char* c);
	// generate bitstream before transmitting
	
};

#endif