#include "Device/Network/DhcpClient.h"
#include "Poco/Thread.h"
int main(int argc, char** argv)
{
	int i = 0;
	CDhcpClient dhcp;
	dhcp.startDhcp("eth0");
	for(i = 0; i < 30; i++)
	{
		printf("waiting %d seconds....\r\n", i+1);
		Poco::Thread::sleep(1000);
	}
	printf("=================================stop dhcp========================\n");
	dhcp.stopDhcp();

	for(i = 0; i < 10; i++)
	{
		printf("waiting %d seconds....\r\n", i+1);
		Poco::Thread::sleep(1000);
	}
	printf("================================start dhcp=======================\n");
	dhcp.startDhcp("eth0");

	for(i = 0; i < 10; i++)
	{
		printf("waiting %d seconds....\r\n", i+1);
		Poco::Thread::sleep(1000);
	}
	printf("=================================stop dhcp========================\n");
	dhcp.stopDhcp();
	return 0;
}

