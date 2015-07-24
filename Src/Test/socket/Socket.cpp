#include <stdio.h>
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
using namespace Poco::Net;
int main(int argc, char** argv)
{
	SocketAddress sa("127.0.0.1", 12222);
	StreamSocket ss(sa);
	char buf[512] = {0, };
	scanf("%s", buf);
	ss.sendBytes(buf, strlen(buf));
	printf("after first send.\n");
	getchar();
	getchar();
	ss.sendBytes(buf, strlen(buf));
	printf("after second send.\n");
	getchar();
	ss.sendBytes(buf, strlen(buf));
	printf("after third send.\n");
	getchar();
	ss.close();
	return 0;
}

