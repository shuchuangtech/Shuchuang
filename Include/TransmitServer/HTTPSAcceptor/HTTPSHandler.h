#ifndef __SERVER_HTTPS_HANDLER_H__
#define __SERVER_HTTPS_HANDLER_H__
#include "Poco/Task.h"
#include "TransmitServer/SocketNode.h"
class CHTTPSHandler : public Poco::Task
{
public:
	CHTTPSHandler();
	//type 0 out socket, type 1 in socket
	bool setParam(int type, SocketNode* sn, int http_port);
	void runTask();
	int getType();
	SocketNode* getSocketNode();
	bool getResult();
private:
	int				m_type;
	int				m_http_port;
	bool			m_result;
	SocketNode*		m_sn;
};
#endif

