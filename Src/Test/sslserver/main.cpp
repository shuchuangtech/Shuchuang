#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Types.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Common/PrintLog.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	UInt16 port = atoi(argv[1]);
/*	ServerSocket tcp_server(port);
	SocketAddress clientsa;
	StreamSocket cs = tcp_server.acceptConnection(clientsa);
	char bbuf[512] = {0, };
	while(1)
	{
		if(cs.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			int aa = cs.available();
			tracef("%s, %d tcp available %d\n", __FILE__, __LINE__, aa);
			memset(bbuf, 0, 512);
			cs.receiveBytes(bbuf, 512);
			tracef("%s, %d: tcp receive %s\n", __FILE__, __LINE__, bbuf);
		}
	}
*/
	Context::Ptr pContext = NULL;
	try
	{
	pContext = new Context(Context::TLSV1_SERVER_USE, 
							"./privkey.pem",
							"./cert.pem",
							"",
							Context::VERIFY_NONE);
	}
	catch(Exception& e)
	{
		errorf("%s, %d: %s\n", __FILE__, __LINE__, e.message().c_str());
	}
	SecureServerSocket* ssl_server = NULL;
	ssl_server = new SecureServerSocket(port, 64, pContext);
	while(1)
	{
		SocketAddress clientAddress;
		SecureStreamSocket ss;
		while(1)
		{
			if(ssl_server->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
			{
				ss = ssl_server->acceptConnection(clientAddress);
				break;
			}
		}
		tracef("%s, %d: Accept connection from %s.\n", __FILE__, __LINE__, clientAddress.toString().c_str());
		char buf[1024] = {0, };
		char t_buf[1024] = {0, };
		while(1)
		{
			int pos = 0;
			if(ss.poll(Timespan(5, 0), Socket::SELECT_READ|Socket::SELECT_ERROR) > 0)
			{
				try
				{
					while(ss.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
					{
						tracepoint();
						int ret = ss.receiveBytes(t_buf, 1024);
						if(ret == 0)
							break;
						snprintf(buf + pos, 1023 - pos, "%s", t_buf);
						pos += ret;
						tracef("%s, %d: receive bytes: %s\n", __FILE__, __LINE__, t_buf);
						memset(t_buf, 0, 1024);
					}
					break;
				}
				catch(Exception& e)
				{
					errorf("%s, %d: message %s\n", __FILE__, __LINE__, e.message().c_str());
				}
			}
		}
		tracef("%s, %d: read %s\n", __FILE__, __LINE__, buf);
		SocketAddress sa("127.0.0.1", 8888);
		StreamSocket sss(sa);
		sss.sendBytes(buf, 1024);
		if(sss.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			memset(buf, 0, 1024);
			sss.receiveBytes(buf, 1024);
			tracef("%s, %d: %s\n", __FILE__, __LINE__, buf);
			ss.sendBytes(buf, 1024);
		}
	}
	delete ssl_server;
	return 0;
}

