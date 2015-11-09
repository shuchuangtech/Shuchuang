#ifndef __DEVICE_RPC_CLIENT_H__
#define __DEVICE_RPC_CLIENT_H__
#include "Poco/Task.h"
#include "Poco/JSON/Object.h"
#include "Device/RequestNotification.h"
class CRPCClient : public Poco::Task
{
public:
	CRPCClient();
	~CRPCClient();
	RequestNotification::Ptr getResult();
	bool setRequest(RequestNotification::Ptr);
	void runTask();
private:
	bool parseAction(std::string& opt, std::string& component, std::string& method);
	RequestNotification::Ptr			m_request;
	RequestNotification::Ptr			m_result;
	int									m_id;
	Poco::JSON::Object::Ptr				m_param;
};
#endif
