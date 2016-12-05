#ifndef __SERVER_H__
#define __SERVER_H__

#include <string>
#include <vector>
#include <openssl/sha.h>

#include "global.h"
#include "Packet.h"

using namespace std;

struct torrent_t {
	char	name[64];
	char	hashcode[SHA_DIGEST_LENGTH * 2];
};

class Server {
	
protected:

	vector<torrent_t> 		torrent_list;
	// store the torrent list for querying. (<torrent filename, sha_1>)
	// need semaphore.

public:

	Server() {}
	~Server() {}
	// constructor and destructor.
	
	bool 			hasName(char* name);
	bool			createTorrent(char* stream);
	// create a torrent from the packet.
	void			onReceiveTorrent(char* stream, char* pkt);
	// decrypt to torrent file.
	// check name uniqueness.
	// save the torrent to the preset path.
	// insert to the map.
	void		 	onRequestTorrentList(char* stream, char* pkt);
	// return all torrents to the peer.
	void			onRequestTorrentSpecified(char* stream, char* pkt);
	// check if received filename match w/some file.
	// if matched, send the torrent file.
	// if not matched, send nak.
	void 			deleteLatestFile();
	// move back the file.

};

#endif