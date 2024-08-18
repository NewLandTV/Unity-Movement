#include <iostream>
#include <process.h>
#include "Server.h"
#include "Command.h"

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	Server* overlappedEvent = (Server*)p;

	overlappedEvent->WorkerThread();

	return 0;
}

Server::Server() : socketInfo(), listenSocket(), iocpHandle(), workerHandle(), accept(true), runningWorkerThread(true), clientPlayers()
{
	system("title Movement Server");
	system("mode con cols=30 lines=9");

	CONSOLE_CURSOR_INFO consoleCursorInfo;

	consoleCursorInfo.bVisible = 0;
	consoleCursorInfo.dwSize = 1;

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleCursorInfo);
}

Server::~Server()
{
	WSACleanup();

	if (socketInfo)
	{
		delete[] socketInfo;

		socketInfo = nullptr;
	}

	if (workerHandle)
	{
		delete[] workerHandle;

		workerHandle = nullptr;
	}

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (clientPlayers[i] != nullptr)
		{
			delete clientPlayers[i];

			clientPlayers[i] = nullptr;
		}
	}
}

bool Server::Initialize(unsigned short port)
{
	// Initialze winsock v2.2
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "Failed to WSAStartup().";

		return false;
	}

	// Socket initialze
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to WSASocket().";

		return false;
	}

	// Server info settings
	SOCKADDR_IN serverAddrIn;

	serverAddrIn.sin_family = PF_INET;
	serverAddrIn.sin_port = htons(port);
	serverAddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// Socket settings
	if (bind(listenSocket, (SOCKADDR*)&serverAddrIn, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cout << "Failed to bind().";

		closesocket(listenSocket);
		WSACleanup();

		return false;
	}

	// Listening socket settings
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		std::cout << "Failed to listen().";

		closesocket(listenSocket);
		WSACleanup();

		return false;
	}

	return true;
}

void Server::Start()
{
	// Client info
	SOCKET clientSocket;
	SOCKADDR_IN clientAddrIn;
	int addrInLength = sizeof(SOCKADDR_IN);
	DWORD recvBytes;
	DWORD flags;

	// Create completion port
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

	// Create worker thread
	if (!CreateWorkerThread())
	{
		return;
	}

	// Accept client
	while (accept)
	{
		clientSocket = WSAAccept(listenSocket, (SOCKADDR*)&clientAddrIn, &addrInLength, nullptr, 0);

		if (clientSocket == INVALID_SOCKET)
		{
			std::cout << "Failed to WSAAccept().";

			return;
		}

		socketInfo = new SocketInfo();
		socketInfo->dataBuffer.len = MAX_BUFFER_SIZE;
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
		socketInfo->socket = clientSocket;
		socketInfo->recvBytes = 0;
		socketInfo->sendBytes = 0;
		flags = 0;

		// Specifies a nested socket and passes a function to be executed upon completion
		iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (DWORD)socketInfo, 0);

		if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &recvBytes, &flags, &(socketInfo->overlapped), nullptr) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "Failed to io pending : " << WSAGetLastError();

			return;
		}
	}
}

bool Server::CreateWorkerThread()
{
	unsigned int threadId;
	SYSTEM_INFO systemInfo;

	GetSystemInfo(&systemInfo);

	int threadCount = systemInfo.dwNumberOfProcessors * 2;

	workerHandle = new HANDLE[threadCount];

	for (int i = 0; i < threadCount; i++)
	{
		workerHandle[i] = (HANDLE*)_beginthreadex(nullptr, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId);

		if (workerHandle[i] == nullptr)
		{
			std::cout << "Failed to create worker thread.";

			return false;
		}

		ResumeThread(workerHandle[i]);
	}

	return true;
}

void Server::WorkerThread()
{
	// Overlapped I/O send & recv size
	DWORD recvBytes;
	DWORD sendBytes;
	DWORD flags = 0;
	SocketInfo* completionKey;
	SocketInfo* socketInfo;

	while (runningWorkerThread)
	{
		// Disconnect client
		if (!GetQueuedCompletionStatus(iocpHandle, &recvBytes, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&socketInfo, INFINITE) && recvBytes == 0)
		{
			closesocket(socketInfo->socket);
			free(socketInfo);

			continue;
		}

		socketInfo->dataBuffer.len = recvBytes;

		if (recvBytes == 0)
		{
			closesocket(socketInfo->socket);
			free(socketInfo);

			continue;
		}

		// Process logic server side
		char buffer[MAX_BUFFER_SIZE];
		char* context = nullptr;
		char* context2 = nullptr;
		char* ptr = strtok_s(socketInfo->dataBuffer.buf, "|", &context);

		switch (atoi(ptr))
		{
		case CMD_C2S_JOIN_GAME:
			if (clientCount >= MAX_CLIENT)
			{
				continue;
			}

			char userId[8];

			clientPlayers[clientCount] = new Player(0, 0);

			sprintf_s(userId, 8, "0|%d", clientCount);
			strcpy_s(buffer, MAX_BUFFER_SIZE, userId);

			clientCount++;

			break;
		case CMD_C2S_LEAVE_GAME:
			ptr = strtok_s(nullptr, "|", &context);

			if (clientPlayers[atoi(ptr)] != nullptr)
			{
				delete clientPlayers[atoi(ptr)];

				clientPlayers[atoi(ptr)] = nullptr;

				clientCount--;
			}

			break;
		case CMD_C2S_PLAYER_MOVEMENT:
			ptr = strtok_s(nullptr, "|", &context);

			char* data = strtok_s(context, ",", &context2);

			unsigned short x = atoi(data);

			data = strtok_s(nullptr, ",", &context2);

			unsigned short y = atoi(data);

			clientPlayers[atoi(ptr)]->SetPosition(x, y);

			break;
		}

		// Check packet size
		if (buffer != nullptr && strlen(buffer) > 0)
		{
			socketInfo->dataBuffer.len = strlen(buffer);
			socketInfo->dataBuffer.buf = buffer;

			// Send to client
			if (WSASend(socketInfo->socket, &(socketInfo->dataBuffer), 1, &sendBytes, flags, nullptr, nullptr) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "Failed to WSASend()." << WSAGetLastError();
			}
		}

		// Initialize socket info
		ZeroMemory(&(socketInfo->overlapped), sizeof(OVERLAPPED));

		socketInfo->dataBuffer.len = MAX_BUFFER_SIZE;
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer;

		ZeroMemory(socketInfo->messageBuffer, MAX_BUFFER_SIZE);

		socketInfo->recvBytes = 0;
		socketInfo->sendBytes = 0;
		flags = 0;

		if (WSARecv(socketInfo->socket, &(socketInfo->dataBuffer), 1, &recvBytes, &flags, (LPWSAOVERLAPPED) & (socketInfo->overlapped), nullptr) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "Failed to WSARecv()." << WSAGetLastError();
		}
	}
}