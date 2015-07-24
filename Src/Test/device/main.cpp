#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Types.h"
#include "Poco/Timespan.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Thread.h"
#include "Common/PrintLog.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	if(argc < 4)
		return 0;
	std::string host = argv[1];
	UInt64 ssl_port = atoi(argv[2]);
	UInt64 reg_port = atoi(argv[3]);
	SocketAddress ssl_addr(host, ssl_port);
	SocketAddress reg_addr(host, reg_port);
	Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	SecureStreamSocket* ssl_sock = new SecureStreamSocket(pContext);
	ssl_sock->connect(ssl_addr, Timespan(3, 0));
	char buf[512] = {0, };
	DynamicStruct ssl_buf;
	ssl_buf["type"] = "request";
	ssl_buf["action"] = "gettoken";
	ssl_buf["key"] = "alpha2015";
	ssl_buf["uuid"] = "hjhjhjhj";
	ssl_buf["devType"] = "abc";
	snprintf(buf, 511, "%s", ssl_buf.toString().c_str());
	ssl_sock->sendBytes(buf, 512);
	memset(buf, 0, 512);
	if(ssl_sock->poll(Timespan(3, 0), Socket::SELECT_READ) > 0)
	{
		ssl_sock->receiveBytes(buf, 512);
	}
	ssl_sock->close();
	delete ssl_sock;
	pContext = NULL;
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(buf);
	JSON::Object::Ptr obj = var.extract<JSON::Object::Ptr>();
	DynamicStruct ssl_recv = *obj;
	std::string token = ssl_recv["token"];
	DynamicStruct reg_buf;
	reg_buf["action"] = "register";
	reg_buf["token"] = token;
	reg_buf["uuid"] = "hjhjhjhj";
	reg_buf["devType"] = "abc";
	memset(buf, 0, 512);
	snprintf(buf, 511, "%s", reg_buf.toString().c_str());
	StreamSocket* reg_sock = new StreamSocket(reg_addr);
	reg_sock->sendBytes(buf, 512);
	memset(buf, 0, 512);
	if(reg_sock->poll(Timespan(3, 0), Socket::SELECT_READ) > 0)
	{
		reg_sock->receiveBytes(buf, 512);
	}
	Thread::sleep(2000);
	while(1)
	{
		if(reg_sock->poll(Timespan(3, 0), Socket::SELECT_READ) > 0)
		{
			memset(buf, 0, 512);
			reg_sock->receiveBytes(buf, 512);
			parser.reset();
			Dynamic::Var recv_request = parser.parse(buf);
			JSON::Object::Ptr req_obj = recv_request.extract<JSON::Object::Ptr>();
			DynamicStruct req_ds = *req_obj;
			tracef("recv request:%s\n", req_ds.toString().c_str());
			req_ds["type"] = "response";
			req_ds["result"] = "good";
			snprintf(buf, 512, "%s", req_ds.toString().c_str());
			reg_sock->sendBytes(buf, 512);
			infof("send response[%s].\n", buf);
			char c;
			c = getchar();
			if(c == 'q')
				break;
		}
	}
	delete reg_sock;
	return 0;
}

