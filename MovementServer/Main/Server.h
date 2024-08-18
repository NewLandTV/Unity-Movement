#ifndef __SERVER_H__
#define __SERVER_H__

#include <deque>
#include <WinSock2.h>
#include "Player.h"

#define MAX_BUFFER_SIZE 2048
#define MAX_CLIENT 100

#pragma comment(lib, "ws2_32.lib")

struct SocketInfo
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	char messageBuffer[MAX_BUFFER_SIZE];
	int recvBytes;
	int sendBytes;
};

class Server
{
private:
	SocketInfo* socketInfo;
	SOCKET listenSocket;
	HANDLE iocpHandle;
	HANDLE* workerHandle;
	Player* clientPlayers[MAX_CLIENT];

	// Flags
	bool accept;
	bool runningWorkerThread;

	unsigned short clientCount;

public:
	Server();
	~Server();

	bool Initialize(unsigned short port);
	void Start();
	bool CreateWorkerThread();
	void WorkerThread();
};

#endif