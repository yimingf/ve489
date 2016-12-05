#include "Packet.h"

void Packet::setrequest(){
	for (int i=0;i<3;i++){
		for (int j=0;j<4;j++){
			request[i][j]=0xff;
		}
	}
}
Packet::Packet(){
	length=17;
	type=DEFAULT;
	setrequest();
}
Packet::Packet(char* chunk){
	char c[4];
	for (int i=0;i<4;i++){
		c[i]=chunk[i];
	}
	length=TransCHAR(c);
	type=(MessageID)chunk[4];
		for (int i=0;i<3;i++){
			for (int j=0;j<4;j++){
				request[i][j]=chunk[5+i*4+j];
			}
		}
	if (length>17){
		payload=chunk+17;
		chunk[length] = '\0';
	}

}
Packet::Packet(MessageID t){
	length=17;
	type=t;
	setrequest();
}
Packet::Packet(MessageID t, char* p, unsigned int plength){
	length=17+plength;
	type=t;
	setrequest();
	payload=p;
}

Packet::Packet(MessageID t, unsigned int r[], char* p, unsigned int plength){
	length=17+plength;
	type=t;
	for (int i=0;i<3;i++){
		TransINT(r[i],request[i]);
	}
	payload=p;
}

Packet::Packet(MessageID t, unsigned int r[]){
	length=17;
	type=t;
	for (int i=0;i<3;i++){
		TransINT(r[i],request[i]);
	}
}

unsigned int Packet::getLength(){
	return length;
}
MessageID Packet::getType(){
	return type;
}
char * Packet::getPayload(){
	return payload;
}
unsigned int Packet::getPlength(){
	return length-17;
}

void Packet::getRequest( unsigned int r[] ) {
	for (int i = 0; i < 3; i++ ){
		r[i] = TransCHAR( request[i] );
	}
	return;
}

void Packet::generateChunk(char* c){
	TransINT(length,c);
	c[4]=type;
	for(int i=0;i<3;i++){
		for (int j=0;j<4;j++){
			c[5+i*4+j]=request[i][j];
		}
	}
	if (length>17){
		for (unsigned int i=0;i<length-17;i++) c[i+17]=payload[i];
	}
}

Packet::~Packet(){}