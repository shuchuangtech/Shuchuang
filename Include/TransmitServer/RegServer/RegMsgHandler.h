#ifndef __SERVER_REG_MSG_HANDLER_H__
#define __SERVER_REG_MSG_HANDLER_H__
#include "Poco/Task.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Timespan.h"
#include "TransmitServer/RegServer/SocketTime.h"
using namespace Poco;
using namespace Poco::Net;
#define SERVER_KEY_STR "alpha2015"
class CRegMsgHandler : public Task
{
public:
	CRegMsgHandler(int type);
	//0 ssl, 1 reg
	~CRegMsgHandler();
	void				runTask();
	bool				setSocket(SocketTime* st);
	int					getType();
	SocketTime*			getSocket();
	UInt64				getRequestID();
	JSON::Object::Ptr	getHTTPResponse();
	bool				socketReceive();
private:
	bool				receiveBytes(Timespan timeout);
	bool				parseRequest(const char* buf, JSON::Object::Ptr& request);
	bool				handleRegMsg(JSON::Object::Ptr request, JSON::Object::Ptr result);
	bool				handleSslMsg(JSON::Object::Ptr request, JSON::Object::Ptr result);
	bool				formatCheck(JSON::Object::Ptr request, JSON::Object::Ptr result, DynamicStruct& param);
	bool				parseAction(std::string action, std::string& component, std::string& method);
	//variables
	bool				m_receive;
	//http request id, id=0 for device register request
	UInt64				m_req_id;
	int					m_type;
	std::string			m_recv_buf;
	SocketTime*			m_socket;
	JSON::Object::Ptr	m_http_response;
};
#endif

