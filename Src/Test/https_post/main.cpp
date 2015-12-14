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
	UInt16 port = atoi(argv[1]);
	Context::Ptr pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE);
	SocketAddress sa("127.0.0.1", port);
	SecureStreamSocket ss1(sa, pContext);
	Thread::sleep(50);
	std::string uri = "";
	uri += "POST / HTTP/1.1\r\n";
	uri += "Host: 127.0.0.1:8888\r\n";
	uri += "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:38.0) Gecko/20100101 Firefox/38.0\r\n";
	uri += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	uri += "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n";
	uri += "Accept-Encoding: gzip, deflate\r\n";
	uri += "Content-Type: application/json; charset=UTF-8\r\n";
	uri += "Content-Length: ";
	DynamicStruct req;
	std::cout << "action:";
	std::string str;
	std::cin >> str;
	req["type"] =  "request";
	req["action"] = str;
	if(str == "user.login")
	{
		std::cout << "username:";
		std::cin >> str;
		DynamicStruct param;
		param["username"] = str;
		std::cout << "uuid:";
		std::cin >> str;
		param["uuid"] = str;
		req["param"] = param;
	}
	int len = req.toString().length();
	char lenstr[5] = {0, };
	snprintf(lenstr, 4, "%d", len);
	uri += lenstr;
	uri += "\r\n";
	uri += "Connection: keep-alive\r\n";
	uri += "Pragma: no-cache\r\n";
	uri += "Cache-Control: no-cache\r\n\r\n";
	uri += req.toString();
	//uri += "\r\n";
	printf("%s\n", uri.c_str());
	ss1.sendBytes(uri.c_str(), uri.length());
	char buf[1024] = {0, };
	if(ss1.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
	{
		ss1.receiveBytes(buf, 1024);
		std::cout << "recv: " << std::endl << buf << std::endl;
	}
	return 0;
}

