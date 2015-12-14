#include "TransmitServer/HTTPServer/HTTPRequestHandler.h"
#include "Common/PrintLog.h"
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
using namespace Poco::Net;
CHTTPRequestHandler::CHTTPRequestHandler()
{
	m_buf = NULL;
}

CHTTPRequestHandler::~CHTTPRequestHandler()
{
	if(m_buf != NULL)
		delete[] m_buf;
}

void CHTTPRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setContentType("application/json");
	response.setChunkedTransferEncoding(true);
	if(m_buf == NULL)
		m_buf = new char[512];
	memset(m_buf, 0, 512);
	request.stream().getline(m_buf, 512, '\n');
	infof("%s, %d: Receive HTTP request[%s]", __FILE__, __LINE__, m_buf);
	DynamicStruct ds;
	ds["attr1"] = "content1";
	ds["attr2"] = "content2";
	infof("%s, %d: Send Http response[%s].", __FILE__, __LINE__, ds.toString().c_str());
	response.sendBuffer(ds.toString().c_str(), ds.toString().length());
}

