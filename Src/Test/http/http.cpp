#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Types.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Thread.h"
#include "Common/PrintLog.h"
using namespace Poco;
using namespace Poco::Net;
class MyRequestHandler : public HTTPRequestHandler
{
public:
	MyRequestHandler(){}
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		char buf[512] = {0, };
		request.stream().getline(buf, 512, '\n');
		tracef("steam %s\n", buf);
		HTTPServerRequest::ConstIterator it = request.begin();
		for(; it != request.end(); it++)
		{
			tracef("%s:%s\n", it->first.c_str(), it->second.c_str());
		}
	}
};
class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	MyRequestHandlerFactory(){}
	~MyRequestHandlerFactory(){}
	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		std::string str = request.getURI();
		tracef("URI:%s\n", str.c_str());
		if(request.getURI() == "/")
			return new MyRequestHandler();
		else
			return 0;
	}
};

int main(int argc, char** argv)
{
	UInt16 port = atoi(argv[1]);
	HTTPServerParams* pParams = new HTTPServerParams;
	pParams->setMaxQueued(100);
	pParams->setMaxThreads(16);
	ServerSocket svs(port);
	HTTPServer svr(new MyRequestHandlerFactory(), svs, pParams);
	svr.start();
	Thread::sleep(600 * 1000);
	svr.stop();
	return 0;
}

