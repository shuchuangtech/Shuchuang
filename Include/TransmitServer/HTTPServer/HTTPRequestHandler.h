#ifndef __SERVER_HTTP_REQUEST_HANDLER_H__
#define __SERVER_HTTP_REQUEST_HANDLER_H__
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Object.h"
class CHTTPRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
	CHTTPRequestHandler();
	~CHTTPRequestHandler();
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
private:
	bool checkRequestFormat(Poco::JSON::Object::Ptr request, Poco::JSON::Object::Ptr response);
	bool parseAction(std::string action, std::string& component, std::string& method);
	char* m_buf;
};
#endif

