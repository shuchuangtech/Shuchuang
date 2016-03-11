#ifndef __DEVICE_MESSAGE_NOTIFICATION_H__
#define __DEVICE_MESSAGE_NOTIFICATION_H__
#include "Poco/Notification.h"
#include "Poco/JSON/Object.h"
class MessageNotification : public Poco::Notification
{
public:
	typedef Poco::AutoPtr<MessageNotification> Ptr;
	MessageNotification();
	~MessageNotification();
	bool setName(const std::string& name);
	bool setParam(const Poco::JSON::Object::Ptr& param);
	std::string& getName();
	Poco::JSON::Object::Ptr& getParam();
private:
	std::string m_name;
	Poco::JSON::Object::Ptr m_param;
};
#endif

