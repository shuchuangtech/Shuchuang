#ifndef __DEVICE_REQUEST_NOTIFICATION_H__
#define __DEVICE_REQUEST_NOTIFICATION_H__
#include "Poco/Notification.h"
#include "Poco/JSON/Object.h"
#include "Poco/Types.h"
class RequestNotification : public Poco::Notification
{
public:
	typedef Poco::AutoPtr<RequestNotification> Ptr;
	RequestNotification(Poco::UInt64, std::string, Poco::JSON::Object::Ptr&);
	~RequestNotification();	
	std::string&				getRequest();
	Poco::JSON::Object::Ptr&	getResponse();
	Poco::UInt64				getID();
private:
	Poco::UInt64			m_id;
	std::string				m_request;
	Poco::JSON::Object::Ptr	m_response;
};
#endif

