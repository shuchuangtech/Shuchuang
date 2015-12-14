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
		pContext = new Context(Context::TLSV1_2_SERVER_USE, 
							"./my.key",
							"./my.crt",
							"",
							Context::VERIFY_NONE,
							9,
							false,
							"ALL:!LOW:!EXP:!MD5:@STRENGTH");
	}
	catch(Exception& e)
	{
		errorf("%s, %d: %s\n", __FILE__, __LINE__, e.message().c_str());
	}
	SecureServerSocket* ssl_server = NULL;
	ssl_server = new SecureServerSocket(port, 64, pContext);
	SocketAddress clientAddress;
	StreamSocket ss;
	while(1)
	{
		if(ssl_server->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			ss = ssl_server->acceptConnection(clientAddress);
			break;
		}
	}
	tracef("%s, %d: Accept connection from %s.\n", __FILE__, __LINE__, clientAddress.toString().c_str());
	while(1)
	{
		if(ss.poll(Timespan(5, 0), Socket::SELECT_READ|Socket::SELECT_ERROR) > 0)
		{
			std::string buf = "";
			char t_buf[512] = {0, };
			ss.setReceiveTimeout(Timespan(10, 0));
			try
			{
				if(ss.receiveBytes(t_buf, 512) <= 0)
				{
					warnf("reveice error.");
					break;
				}
				else
				{
					buf += t_buf;
					while(ss.available())
					{
						memset(t_buf, 0, 512);
						if(ss.receiveBytes(t_buf, 512) > 0)
						{
							buf += t_buf;
						}
					}
				}
			}
			catch(Exception& e)
			{
				errorf("%s, %d: message %s\n", __FILE__, __LINE__, e.message().c_str());
				break;
			}
			infof("receive buf : %s", buf.c_str());
		}
	}
	delete ssl_server;
	return 0;
}

