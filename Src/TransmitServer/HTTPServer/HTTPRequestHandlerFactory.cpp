#include "TransmitServer/HTTPRequestHandlerFactory.h"
#include "TransmitServer/HTTPRequestHandler.h"
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

