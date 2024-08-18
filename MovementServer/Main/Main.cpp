#include "Server.h"

int main()
{
	Server server = Server();

	if (server.Initialize(2714))
	{
		server.Start();
	}

	return 0;
}