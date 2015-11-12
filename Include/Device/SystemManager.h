#ifndef __DEVICE_SYSTEM_MANAGER_H__
#define __DEVICE_SYSTEM_MANAGER_H__
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/SingletonHolder.h"
#include "Device/Network/NetworkManager.h"
#include <map>
class CSystemManager : public Poco::Runnable
{
public:
	CSystemManager();
	~CSystemManager();
	static CSystemManager* instance()
	{
		static Poco::SingletonHolder<CSystemManager> sh;
		return sh.get();
	}
	bool start();
	bool stop();
	void run();
private:
	Poco::Thread		m_thread;
	bool				m_started;
	CNetworkManager*	m_network;
	std::map<std::string, bool>		m_dhcp_state;	
};
#endif

