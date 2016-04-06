#ifndef __SERVER_DEVICE_MANAGER_H__
#define __SERVER_DEVICE_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/Mutex.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include "TransmitServer/APNS/ApplePush.h"
#include <map>
struct _DeviceInfo
{
	_DeviceInfo(std::string u, Poco::UInt64 i, std::string type, std::string t, std::string m)
		:uuid(u), id(i), devType(type), token(t), bindMobileToken(m)
	{
		online = false;
	}
	std::string			uuid;
	Poco::UInt64		id;
	std::string			devType;
	std::string			token;
	std::string			bindMobileToken;
	Poco::Timestamp		time;
	bool				online;
};
typedef struct _DeviceInfo DeviceInfo;
class CDeviceManager : public Poco::Runnable
{
public:
	CDeviceManager();
	~CDeviceManager();
	static CDeviceManager* instance()
	{
		static Poco::SingletonHolder<CDeviceManager> sh;
		return sh.get();
	}
	bool start();
	bool stop();
	bool addObserver(const Poco::AbstractObserver& o);
	bool removeObserver(const Poco::AbstractObserver& o);
	void run();
	bool addDevice(const std::string uuid, const Poco::UInt64 id, const std::string devType, std::string& token, const std::string& mobile_token);
	bool keepAliveDevice(const std::string uuid);
	bool deviceOnline(const std::string uuid, const std::string token, Poco::UInt64 sock_id);
	bool deviceOffline(const std::string uuid);
	bool deviceOffline(const Poco::UInt64 id);
	bool checkDevice(const std::string uuid);
	bool bindMobile(const std::string uuid, const std::string token);
	bool unbindMobile(const std::string uuid);
	DeviceInfo* getDevice(const std::string uuid);
private:
	bool											m_started;
	CApplePush*										m_apns;
	Poco::NotificationCenter*						m_noti_center;
	Poco::Thread									m_thread;
	Poco::Mutex										m_mutex;
	std::map<std::string, DeviceInfo*>				m_device_map;
	std::map<Poco::UInt64, std::string>				m_device_id_map;
	typedef std::map<std::string, DeviceInfo*>::iterator DeviceMapIt;
	typedef std::map<Poco::UInt64, std::string>::iterator DeviceIdMapIt;
};
#endif

