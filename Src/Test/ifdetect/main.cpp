#include "Device/NetworkManager.h"
#include "stdio.h"
int main(int argc, char** argv)
{
	bool isUp;
	CNetworkManager::getMiiLinkState("eth0", isUp);
	printf("eth up: %s\n", isUp?"up":"down");
	return 0;
}

