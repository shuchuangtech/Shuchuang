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
#include <map>
using namespace Poco;
struct _DeviceInfo
{
	_DeviceInfo(std::string u, int i, std::string type, std::string t)
		:uuid(u), id(i), devType(type), token(t)
	{
		online = false;
	}
	std::string		uuid;
	UInt64			id;
	std::string		devType;
	std::string		token;
	Timestamp		t;
	bool			online;
};
typedef struct _DeviceInfo DeviceInfo;
class CDeviceManager : public Runnable
{
public:
	CDeviceManager();
	~CDeviceManager();
	static CDeviceManager* instance()
	{
		static SingletonHolder<CDeviceManager> sh;
		return sh.get();
	}
	bool start();
	bool stop();
	bool addObserver(const AbstractObserver& o);
	bool removeObserver(const AbstractObserver& o);
	void run();
	bool addDevice(const std::string uuid, const UInt64 id, const std::string devType, std::string& token);
	bool keepAliveDevice(const std::string uuid);
	bool deviceOnline(const std::string uuid, const std::string token, UInt64 sock_id);
	bool deviceOffline(const std::string uuid);
	bool deviceOffline(const UInt64 id);
	bool checkDevice(const std::string uuid);
	DeviceInfo* getDevice(const std::string uuid);
private:
	bool								m_started;
	NotificationCenter*					m_noti_center;
	Thread								m_thread;
	Mutex								m_mutex;
	std::map<std::string, DeviceInfo*>	m_device_map;
	std::map<UInt64, std::string>			m_device_id_map;
	typedef std::map<std::string, DeviceInfo*>::iterator DeviceMapIt;
	typedef std::map<UInt64, std::string>::iterator DeviceIdMapIt;
};
#endif

