#ifndef __DEVICE_REQUEST_NOTIFICATION_H__
#define __DEVICE_REQUEST_NOTIFICATION_H__
#include "Poco/Notification.h"
#include "Poco/JSON/Object.h"
class RequestNotification : public Poco::Notification
{
public:
	typedef Poco::AutoPtr<RequestNotification> Ptr;
	RequestNotification(int, Poco::JSON::Object::Ptr);
	~RequestNotification();	
	Poco::JSON::Object::Ptr getParam();
	int getID();
	int setParam(Poco::JSON::Object::Ptr param);
private:
	int						m_id;
	Poco::JSON::Object::Ptr	m_param;
};
#endif

