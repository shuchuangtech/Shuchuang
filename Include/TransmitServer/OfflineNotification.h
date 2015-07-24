#ifndef __SERVER_OFFLINE_NOTIFICATION_H__
#define __SERVER_OFFLINE_NOTIFICATION_H__
#include "Poco/Notification.h"
using namespace Poco;
class OfflineNotification : public Notification
{
public:
	typedef AutoPtr<OfflineNotification> Ptr;
	OfflineNotification(UInt64 id, std::string uuid);
	~OfflineNotification();
	UInt64 getID();
	std::string getUUID();
private:
	UInt64				m_id;
	std::string			m_uuid;
};
#endif

