#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Thread.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Types.h"
#include <iostream>
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	UInt16 port = 0;
	std::string host = "127.0.0.1";
	if(argc == 2)
	{
		port = atoi(argv[1]);
	}
	else if(argc == 3)
	{
		host = argv[1];
		port = atoi(argv[2]);
	}
	else
	{
		printf("error arg.\n");
		return 0;
	}
	std::string ciphers = "";
	ciphers = "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM";
	Context::Ptr pContext = NULL;
	try
	{
		pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, ciphers);
	}
	catch(Exception& e)
	{
		printf("context exception message : %s\n", e.message().c_str());
		return 0;
	}
	SocketAddress sa(host, port);
	SecureStreamSocket* ss1 = NULL;
	try
	{
		ss1 = new SecureStreamSocket(sa, pContext);
	}
	catch(Exception& e)
	{
		printf("SecureStreamSocket context exception: %s\n", e.message().c_str());
		if(ss1 != NULL)
			delete ss1;
		return 0;
	}



	Thread::sleep(50);
	std::string uri = "";
	uri += "GET / HTTP/1.1\r\n";
	uri += "Host: 127.0.0.1:";
	uri += argv[1];
	uri += "\r\n";
	uri += "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:38.0) Gecko/20100101 Firefox/38.0\r\n";
	uri += "Accept: */*\r\n";
	uri += "Accept-Language: en-US\r\n";
	uri += "Accept-Encoding: gzip, deflate\r\n";
	uri += "Connection: keep-alive\r\n";
	printf("%s\n", uri.c_str());
	ss1->sendBytes(uri.c_str(), uri.length());
	char t_buf[512] = {0, };
	std::string buf = "";
	if(ss1->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
	{
		ss1->receiveBytes(t_buf, 512);
		buf += t_buf;
		while(ss1->available())
		{
			memset(t_buf, 0, 512);
			ss1->receiveBytes(t_buf, 512);
			buf += t_buf;
		}
	}
	std::cout << "recv: " << std::endl << buf << std::endl;
	return 0;
}

