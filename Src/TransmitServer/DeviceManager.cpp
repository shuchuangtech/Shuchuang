#include "TransmitServer/DeviceManager.h"
#include "TransmitServer/OfflineNotification.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/MD5Engine.h"
#include "Common/PrintLog.h"
CDeviceManager::CDeviceManager()
:m_thread("DeviceManager")
{
	m_started = false;
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
	m_thread.start(*this);
	m_started = true;
	infof("%s, %d: Device manager start successfully.", __FILE__, __LINE__);
	return true;
}

bool CDeviceManager::stop()
{
	if(!m_started)
		return false;
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
			if(now - di->time > 6 * checkPeriod)
			{
				m_device_map.erase(itemp);
				m_device_id_map.erase(di->id);
				m_noti_center->postNotification(new OfflineNotification(di->id, di->uuid));
				delete di;
				infof("%s, %d: Device[%s：%lu] offline.", __FILE__, __LINE__, di->uuid.c_str(), di->id);
			}
		}
		m_mutex.unlock();
	}
}

bool CDeviceManager::addDevice(const std::string uuid, UInt64 id, const std::string devType, std::string& token)
{
	DeviceMapIt it;
	m_mutex.lock();
	it = m_device_map.find(uuid);
	if(it != m_device_map.end())
	{
		token = it->second->token;
		Timestamp now;
		it->second->time = now;
		m_mutex.unlock();
		return true;
	}
	UUIDGenerator gen;
	UUID uid = gen.create();
	MD5Engine md5;
	md5.update(uid.toString());
	const DigestEngine::Digest& digest = md5.digest();
	token = DigestEngine::digestToHex(digest);
	DeviceInfo* di = new DeviceInfo(uuid, id, devType, token);
	m_device_map.insert(std::make_pair<std::string, DeviceInfo*>(uuid, di));
	infof("%s, %d: Device added[%s:%s]", __FILE__, __LINE__, uuid.c_str(), token.c_str());
	m_mutex.unlock();
	return true;
}

bool CDeviceManager::keepAliveDevice(const std::string uuid)
{
	m_mutex.lock();
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		m_mutex.unlock();
		return false;
	}
	if(!it->second->online)
	{
		m_mutex.unlock();
		return false;
	}
	Timestamp time;
	it->second->time = time;
	infof("%s, %d: Device[%s] keepalive successfully.", __FILE__, __LINE__, uuid.c_str());
	m_mutex.unlock();
	return true;
}

bool CDeviceManager::deviceOnline(const std::string uuid, const std::string token, UInt64 sock_id)
{
	m_mutex.lock();
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		m_mutex.unlock();
		return false;
	}
	if(it->second->online)
	{
		Timestamp now;
		it->second->time = now;
		m_mutex.unlock();
		return true;
	}
	if(token != it->second->token)
	{
		warnf("%s, %d: Device[%s] register token not match.", __FILE__, __LINE__, uuid.c_str());
		m_mutex.unlock();
		return false;
	}
	it->second->online = true;
	it->second->id = sock_id;
	m_device_id_map.insert(std::make_pair<UInt64, std::string>(sock_id, uuid));
	infof("%s, %d: Device[%s] online.", __FILE__, __LINE__, uuid.c_str());
	m_mutex.unlock();
	return true;
}

bool CDeviceManager::deviceOffline(const std::string uuid)
{
	m_mutex.lock();
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		m_mutex.unlock();
		return true;
	}
	delete it->second;
	UInt64 id = it->second->id;
	m_device_map.erase(it);
	DeviceIdMapIt it_id = m_device_id_map.find(id);
	if(it_id == m_device_id_map.end())
	{
		m_mutex.unlock();
		return true;
	}
	m_device_id_map.erase(it_id);
	tracef("%s, %d: Device offline[%s:%lu]", __FILE__, __LINE__, uuid.c_str(), id);
	m_mutex.unlock();
	return true;
}

bool CDeviceManager::deviceOffline(const UInt64 id)
{
	m_mutex.lock();
	DeviceIdMapIt it = m_device_id_map.find(id);
	if(it == m_device_id_map.end())
	{
		m_mutex.unlock();
		return true;
	}
	std::string uuid = it->second;
	m_device_id_map.erase(it);
	DeviceMapIt it_dev = m_device_map.find(uuid);
	if(it_dev == m_device_map.end())
	{
		m_mutex.unlock();
		return true;
	}
	delete it_dev->second;
	m_device_map.erase(it_dev);
	tracef("%s, %d: Device offline[%s]", __FILE__, __LINE__, uuid.c_str());
	m_mutex.unlock();
	return true;
}

bool CDeviceManager::checkDevice(const std::string uuid)
{
	m_mutex.lock();
	DeviceMapIt it = m_device_map.find(uuid);
	if(it == m_device_map.end())
	{
		m_mutex.unlock();
		return false;
	}
	if(!it->second->online)
	{
		m_mutex.unlock();
		return false;
	}
	m_mutex.unlock();
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

