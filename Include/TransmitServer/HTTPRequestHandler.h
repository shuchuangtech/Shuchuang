#ifndef __SERVER_HTTP_REQUEST_HANDLER_H__
#define __SERVER_HTTP_REQUEST_HANDLER_H__
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"
using namespace Poco;
using namespace Poco::Net;
class CHTTPRequestHandler : public HTTPRequestHandler
{
public:
	CHTTPRequestHandler();
	~CHTTPRequestHandler();
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
private:
	bool checkRequestFormat(JSON::Object::Ptr request, JSON::Object::Ptr response);
	bool parseAction(std::string action, std::string& component, std::string& method);
	char* m_buf;
};
#endif

