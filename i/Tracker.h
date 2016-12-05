#ifndef __TRACKER_H__
#define __TRACKER_H__

#include "global.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Packet.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdio>

using namespace std;

class Tracker {

protected:

	vector<torrent_info> peer_list;

public:

	Tracker();
	~Tracker();
	// constructor and destructor.

	void onPeerRegister(char* stream,
		ClientSocket* csocket, char* peerlistpkt);
};

#endif