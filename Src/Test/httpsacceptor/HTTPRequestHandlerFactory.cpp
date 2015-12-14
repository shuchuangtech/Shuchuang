#include "TransmitServer/HTTPServer/HTTPRequestHandlerFactory.h"
#include "TransmitServer/HTTPServer/HTTPRequestHandler.h"
using namespace Poco;
using namespace Poco::Net;
CHTTPRequestHandlerFactory::CHTTPRequestHandlerFactory()
{
}

CHTTPRequestHandlerFactory::~CHTTPRequestHandlerFactory()
{
}

HTTPRequestHandler* CHTTPRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	if(request.getURI() == "/")
		return new CHTTPRequestHandler();
	else
		return 0;
}

