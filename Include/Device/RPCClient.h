#ifndef __DEVICE_RPC_CLIENT_H__
#define __DEVICE_RPC_CLIENT_H__
#include "Poco/Task.h"
#include "Poco/JSON/Object.h"
#include "Poco/Types.h"
class CRPCClient : public Poco::Task
{
public:
	CRPCClient();
	~CRPCClient();
	Poco::JSON::Object::Ptr&	getResponse();
	bool						setRequest(std::string&);
	bool						setID(Poco::UInt64	id);
	Poco::UInt64				getID();
	void						runTask();
private:
	bool parseAction(std::string& opt, std::string& component, std::string& method);
	Poco::UInt64						m_id;
	std::string							m_request_str;
	Poco::JSON::Object::Ptr				m_response;
};
#endif
