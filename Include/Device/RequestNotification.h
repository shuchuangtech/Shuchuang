#ifndef __DEVICE_REQUEST_NOTIFICATION_H__
#define __DEVICE_REQUEST_NOTIFICATION_H__
#include "Poco/Notification.h"
#include "Poco/JSON/Object.h"
using namespace Poco;
class RequestNotification : public Notification
{
public:
	typedef AutoPtr<RequestNotification> Ptr;
	RequestNotification(int, JSON::Object::Ptr);
	~RequestNotification();	
	JSON::Object::Ptr getParam();
	int getID();
	int setParam(JSON::Object::Ptr param);
private:
	int					m_id;
	JSON::Object::Ptr	m_param;
};
#endif

