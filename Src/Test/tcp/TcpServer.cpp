#include <stdio.h>
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Timespan.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	SocketAddress sa("127.0.0.1", 12222);
	ServerSocket ss(sa);
	StreamSocket sss;
	SocketAddress clientAddress;
	while(1)
	{
		if(ss.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			sss = ss.acceptConnection(clientAddress);
			break;
		}
	}
	printf("accept connection from %s.\n", clientAddress.toString().c_str());
	Timespan ts(5, 0);
	char buf[512] = {0, };
	
	Socket::SocketList readList;
	readList.clear();
	Socket::SocketList writeList;
	writeList.clear();
	Socket::SocketList errorList;
	errorList.clear();
	
	for(;;)
	{
		readList.clear();
		errorList.clear();
		readList.push_back(sss);
		errorList.push_back(sss);
		
		if (Socket::select(readList, writeList, errorList, ts) > 0)
		{
			printf("read %d, write %d, error %d\n", readList.size(), writeList.size(), errorList.size());
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				StreamSocket ssss(*it);
				if(ssss.available())
				{
					ssss.receiveBytes(buf, sizeof(buf));
					printf("buf: %s\n", buf);			
				}
				else
				{
					goto endMain;
				}
			}
		}
	}
endMain:
	sss.close();
	ss.close();
}

