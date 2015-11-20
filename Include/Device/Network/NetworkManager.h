#ifndef __DEVICE_NETWORK_MANAGER_H__
#define __DEVICE_NETWORK_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Device/Network/DhcpClient.h"
#include <map>
class CNetworkManager
{
public:
	static bool getMiiLinkState(const char* ifname, bool& isUp);
	static CNetworkManager* instance()
	{
		static Poco::SingletonHolder<CNetworkManager> sh;
		return sh.get();
	}
	CNetworkManager();
	~CNetworkManager();
	bool startDhcp(const char* ethname);
	bool stopDhcp(const char* ethname);
private:
	std::string								m_hostname;
	std::map<std::string, CDhcpClient*>		m_dhcp_map;
};
#endif

