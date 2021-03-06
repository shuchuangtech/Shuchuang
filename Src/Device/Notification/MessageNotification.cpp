#include "Device/Notification/MessageNotification.h"
using namespace Poco;
MessageNotification::MessageNotification()
{
	m_name = "";
	m_param = NULL;
}

MessageNotification::MessageNotification(const std::string& name, const JSON::Object::Ptr& param)
{
	m_name = name;
	m_param = param;
}

MessageNotification::~MessageNotification()
{
	m_param = NULL;
}

bool MessageNotification::setName(const std::string& name)
{
	m_name = name;
	return true;
}

bool MessageNotification::setParam(const JSON::Object::Ptr& param)
{
	m_param = param;
	return true;
}

std::string& MessageNotification::getName()
{
	return m_name;
}

JSON::Object::Ptr& MessageNotification::getParam()
{
	return m_param;
}
