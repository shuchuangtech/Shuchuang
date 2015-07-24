#include "TransmitServer/OfflineNotification.h"
OfflineNotification::OfflineNotification(UInt64 id, std::string uuid)
{
	m_id = id;
	m_uuid = uuid;
}

OfflineNotification::~OfflineNotification()
{
}

UInt64 OfflineNotification::getID()
{
	return m_id;
}

std::string OfflineNotification::getUUID()
{
	return m_uuid;
}
