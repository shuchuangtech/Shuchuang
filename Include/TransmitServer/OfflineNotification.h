#ifndef __SERVER_OFFLINE_NOTIFICATION_H__
#define __SERVER_OFFLINE_NOTIFICATION_H__
#include "Poco/Notification.h"
class OfflineNotification : public Poco::Notification
{
public:
	typedef Poco::AutoPtr<OfflineNotification> Ptr;
	OfflineNotification(Poco::UInt64 id, std::string uuid);
	~OfflineNotification();
	Poco::UInt64 getID();
	std::string getUUID();
private:
	Poco::UInt64		m_id;
	std::string			m_uuid;
};
#endif

