#ifndef __SERVER_REG_MSG_HANDLER_H__
#define __SERVER_REG_MSG_HANDLER_H__
#include "Poco/Task.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/StreamSocket.h"
#include "TransmitServer/RequestInfo.h"
using namespace Poco;
using namespace Poco::Net;
#define SERVER_KEY_STR alpha2015
class CRegMsgHandler : public Task
{
public:
	CRegMsgHandler(int type);
	//0 ssl, 1 reg
	~CRegMsgHandler();
	void runTask();
	bool setParam(UInt64 id, const char* param, int paramLenth, StreamSocket sock);
	bool setRequestInfo(RequestInfo* request);
	int getId();
	JSON::Object::Ptr getResult();
private:
	int m_type;
	int m_id;
	JSON::Object::Ptr m_param;
	JSON::Object::Ptr m_result;
	char* m_buf;
	StreamSocket* m_socket;
};
#endif

