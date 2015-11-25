#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Timespan.h"
#include <stdio.h>
#include "Poco/Thread.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	ServerSocket ss(12222);
	SocketAddress clientAddress;
	StreamSocket sock;
	while(1)
	{
		if(ss.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			sock = ss.acceptConnection(clientAddress);
			break;
		}
	}
	printf("receive connection:%s\n", clientAddress.toString().c_str());
	char buf[1024];
	std::string buffer = "";
	int index = 0;
	while(1)
	{
		if(sock.available())
		{
			printf("available\n");
		}
		else
		{
			printf("unavailable\n");
			if(index != 0)
				break;
			Thread::sleep(500);
			continue;
		}
		if(sock.poll(Timespan(0,100), Socket::SELECT_READ) > 0)
		{
			sock.setReceiveTimeout(10);
			try
			{
				memset(buf, 0, 1024);
				if(sock.receiveBytes(buf, 1024) <= 0)
				{
					printf("disconnect.\n");
					break;
				}
				buffer += buf;
				index++;
			}
			catch(Exception& e)
			{
				printf("error.%s\n", e.message().c_str());
				break;
			}
		}
		else
		{
			printf("receive finished.\n");
			break;
		}
	}
	printf("index:%d\n", index);
	printf("buffer length:%d content:\n%s\n", buffer.length(), buffer.c_str());
	return 0;
}

