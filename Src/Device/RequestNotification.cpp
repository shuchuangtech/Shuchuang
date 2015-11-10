#include "Device/RequestNotification.h"
#include "Common/PrintLog.h"
using namespace Poco;
RequestNotification::RequestNotification(UInt64 id, std::string request, JSON::Object::Ptr& response)
{
	m_id = id;
	m_request = request;
	m_response = response;
}

RequestNotification::~RequestNotification()
{
}

std::string& RequestNotification::getRequest()
{
	return m_request;
}

UInt64 RequestNotification::getID()
{
	return m_id;
}

JSON::Object::Ptr& RequestNotification::getResponse()
{
	return m_response;
}

