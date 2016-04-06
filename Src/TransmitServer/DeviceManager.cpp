#include "TransmitServer/DeviceManager.h"
#include "TransmitServer/OfflineNotification.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/MD5Engine.h"
#include "Common/PrintLog.h"
using namespace Poco;
CDeviceManager::CDeviceManager()
:m_thread("DeviceManager")
{
	m_started = false;
	m_apns = NULL;
	m_noti_center = new NotificationCenter();
}

CDeviceManager::~CDeviceManager()
{
	delete m_noti_center;
}

bool CDeviceManager::addObserver(const AbstractObserver& o)
{
	m_noti_center->addObserver(o);
	return true;
}

bool CDeviceManager::removeObserver(const AbstractObserver& o)
{
	m_noti_center->removeObserver(o);
	return true;
}

bool CDeviceManager::start()
{
	if(m_started)
	{
		warnf("%s, %d: Device manager has already started.", __FILE__, __LINE__);
		return false;
	}
	m_apns = CApplePush::instance();
	m_apns->init();
	m_started = true;
	m_thread.start(*this);
	infof("%s, %d: Device manager start successfully.", __FILE__, __LINE__);
	return true;
}

bool CDeviceManager::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: Device manager not started.", __FILE__, __LINE__);
		return false;
	}
	m_started = false;
	if(m_thread.isRunning())
	{
		m_thread.join();
	}
	tracef("%s, %d: Device manager clearing device info...", __FILE__, __LINE__);
	for(DeviceMapIt it = m_device_map.begin(); it != m_device_map.end(); )
	{
		delete it->second;
		it->second = NULL;
		m_device_map.erase(it++);
	}
	infof("%s, %d: Device manager stop successfully.", __FILE__, __LINE__);
	return true;
}

void CDeviceManager::run()
{
	Timestamp::TimeDiff checkPeriod = 5 * 1000 * 1000;
	while(m_started)
	{
		Thread::sleep(checkPeriod/1000);
		Timestamp now;
		m_mutex.lock();
		for(DeviceMapIt it = m_device_map.begin(); it != m_device_map.end(); )
		{
			DeviceMapIt itemp = it++;
			DeviceInfo* di = itemp->second;
			if(now - di->time > 24 * checkPeriod)
			{
				m_device_map.erase(itemp);
				m_device_id_map.erase(di->id);
				//post notification to RegServer
				m_noti_center->postNotification(new OfflineNotification(di->id, di->uuid));
				infof("%s, %d: Device[%s：%llu] offline.", __FILE__, __LINE__, di->uuid.c_str(), di->id);
				delete di;
			}
		}
		m_mutex.unlock();
	}
}

bool CDeviceManager::addDevice(const std::string uuid, UInt64 id, const std::string devType, std::string& token, const std::string& mobile_token)
{
	DeviceMapIt it;
	Mutex::ScopedLock lock(m_mutex);
	it = m_device_map.find(uuid);
	if(it != m_device_map.end())
	{
		DeviceInfo* d = it->second;
		if(d->online)
		{
			warnf("%s, %d: Device[%s] already online", __FILE__, __LINE__, uuid.c_str());
			return false;
		}
		token = it->second->token;
		Timestamp now;
		it->second->time = now;
		return true;
	}
	UUIDGenerator gen;
	UUID uid = gen.create();
	MD5Engine md5;
	md5.update(uid.toString());
	const DigestEngine::Digest& digest = md5.digest();
	token = DigestEngine::digestToHex(digest);
	DeviceInfo* di = new DeviceInfo(uuid, id, devType, token, mobile_token);
	m_device_map.insert(std::make_pair<std::string, DeviceInfo*>(uuid, di));
	infof("%s, %d: Device added[%s:%s]", __FILE__, __LINE__, uuid.c_str(), token.c_str());
	return true;
}

bool CDeviceManager::keepAliveDevice(const std::string uuid)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return false;
	}
	if(!it->second->online)
	{
		return false;
	}
	Timestamp time;
	it->second->time = time;
	infof("%s, %d: Device[%s] keepalive successfully.", __FILE__, __LINE__, uuid.c_str());
	return true;
}

bool CDeviceManager::deviceOnline(const std::string uuid, const std::string token, UInt64 sock_id)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return false;
	}
	if(it->second->online)
	{
		Timestamp now;
		it->second->time = now;
		return true;
	}
	if(token != it->second->token)
	{
		warnf("%s, %d: Device[%s] register token not match.", __FILE__, __LINE__, uuid.c_str());
		return false;
	}
	it->second->online = true;
	it->second->id = sock_id;
	m_device_id_map.insert(std::make_pair<UInt64, std::string>(sock_id, uuid));
	std::string mobile_token = it->second->bindMobileToken;
	if(!mobile_token.empty())
	{
		std::string content = "您的设备" + it->second->uuid + "已经上线";
		m_apns->pushBmob(mobile_token, content);
	}
	infof("%s, %d: Device[%s] online.", __FILE__, __LINE__, uuid.c_str());
	return true;
}

bool CDeviceManager::deviceOffline(const std::string uuid)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return true;
	}
	std::string mobile_token = it->second->bindMobileToken;
	delete it->second;
	UInt64 id = it->second->id;
	m_device_map.erase(it);
	DeviceIdMapIt it_id = m_device_id_map.find(id);
	if(it_id == m_device_id_map.end())
	{
		return true;
	}
	m_device_id_map.erase(it_id);
	if(!mobile_token.empty())
	{
		std::string push = "您的设备" + uuid + "已离线， 如有异常请及时查看";
		m_apns->pushBmob(mobile_token, push);
	}
	infof("%s, %d: Device offline[%s:%lu]", __FILE__, __LINE__, uuid.c_str(), id);
	return true;
}

bool CDeviceManager::deviceOffline(const UInt64 id)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceIdMapIt it = m_device_id_map.find(id);
	if(it == m_device_id_map.end())
	{
		return true;
	}
	std::string uuid = it->second;
	m_device_id_map.erase(it);
	DeviceMapIt it_dev = m_device_map.find(uuid);
	if(it_dev == m_device_map.end())
	{
		return true;
	}
	std::string mobile_token = it_dev->second->bindMobileToken;
	delete it_dev->second;
	m_device_map.erase(it_dev);
	if(!mobile_token.empty())
	{
		std::string push = "您的设备" + uuid + "已离线， 如有异常请及时查看";
		m_apns->pushBmob(mobile_token, push);
	}
	infof("%s, %d: Device offline[%s]", __FILE__, __LINE__, uuid.c_str());
	return true;
}

bool CDeviceManager::bindMobile(const std::string uuid, const std::string mobile)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return false;
	}
	it->second->bindMobileToken = mobile;
	return true;
}

bool CDeviceManager::unbindMobile(const std::string uuid)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return false;
	}
	it->second->bindMobileToken = "";
	return true;
}

bool CDeviceManager::checkDevice(const std::string uuid)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return false;
	}
	if(!it->second->online)
	{
		return false;
	}
	return true;
}

DeviceInfo* CDeviceManager::getDevice(const std::string uuid)
{
	Mutex::ScopedLock lock(m_mutex);
	DeviceMapIt it;
	it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		return NULL;
	}
	else
	{
		return it->second;
	}
}

