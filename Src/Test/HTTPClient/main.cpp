#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/MediaType.h"
#include "Poco/Types.h"
#include "Poco/Dynamic/Struct.h"
#include "Common/PrintLog.h"
#include <iostream>
using namespace Poco;
using namespace Poco::Net;
std::string g_host;
std::string g_buf;
int g_port;
bool sendRequest(std::string content)
{
	tracef("%s, %d: SendRequest:%s", __FILE__, __LINE__, content.c_str());
	Context::Ptr pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, "EECDH+aRSA+AESGCM");
	HTTPSClientSession https(g_host, (UInt16)g_port, pContext);
	HTTPRequest request;
	request.setContentType(MediaType("application", "json"));
	request.setKeepAlive(true);
	request.setContentLength(content.length());
	std::ostream& ostr = https.sendRequest(request);
	ostr << content << std::flush;
	HTTPResponse response;
	https.setTimeout(Timespan(30, 0));
	try
	{
		std::istream& istr = https.receiveResponse(response);
		g_buf = "";
		char buf[1024] = {0, };
		while(!istr.eof())
		{
			istr.read(buf, 1024);
			g_buf += buf;
			memset(buf, 0, 1024);
		}
	}
	catch(Exception& e)
	{
		warnf("%s, %d: %s", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	tracef("%s, %d: Receive buf length:%d, content:\n%s", __FILE__, __LINE__, g_buf.length(), g_buf.c_str());
	return true;
}

int main(int argc, char** argv)
{
	g_host = argv[1];
	g_port = atoi(argv[2]);
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "server.check";
	DynamicStruct param;
	param["uuid"] = "SC0000000002";
	ds["param"] = param;
	sendRequest(ds.toString());
	return 0;
}

