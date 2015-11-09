#include "Device/RequestNotification.h"
#include "Common/PrintLog.h"
using namespace Poco;
RequestNotification::RequestNotification(int id, JSON::Object::Ptr param)
{
	m_id = id;
	m_param = param;
}

RequestNotification::~RequestNotification()
{
}

JSON::Object::Ptr RequestNotification::getParam()
{
	return m_param;
}

int RequestNotification::getID()
{
	return m_id;
}

int RequestNotification::setParam(JSON::Object::Ptr param)
{
	m_param = param;
	return true;
}

