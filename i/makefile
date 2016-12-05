main : Tracker.cpp tracker_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp global.h Server.cpp server_main.cpp Peer.cpp client_main.cpp File.cpp
	g++ -o ./tracker/tracker Tracker.cpp tracker_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp -std=c++11 -lcrypto -lpthread
	g++ -o ./server/server Server.cpp server_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp -std=c++11 -lcrypto -lpthread
	g++ -o ./peer_up/peer Peer.cpp client_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp File.cpp -std=c++11 -lcrypto -lpthread
	g++ -o ./peer_down/peer Peer.cpp client_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp File.cpp -std=c++11 -lcrypto -lpthread
	g++ -o ./peer_up_2/peer Peer.cpp client_main.cpp Packet.cpp global.cpp ClientSocket.cpp ServerSocket.cpp File.cpp -std=c++11 -lcrypto -lpthread