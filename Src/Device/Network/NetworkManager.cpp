#include "Device/Network/NetworkManager.h"
#include "Common/ConfigManager.h"
#include <string.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Common/PrintLog.h"
#include "Poco/Types.h"
#include "Poco/JSON/Object.h"
CNetworkManager::CNetworkManager()
{
	CConfigManager* config = CConfigManager::instance();
	Poco::JSON::Object::Ptr pConfig;
	config->getConfig("DeviceInfo", pConfig);
	m_hostname = pConfig->getValue<std::string>("uuid");
}

CNetworkManager::~CNetworkManager()
{
}

bool CNetworkManager::getMiiLinkState(const char* ifname, bool& isUp)
{
	int sockfd = 0;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		return false;
	}
	struct ifreq ifr;
	struct mii_ioctl_data* mii = NULL;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	mii = (struct mii_ioctl_data*)&ifr.ifr_data;
	mii->reg_num = MII_BMSR;
	if(ioctl(sockfd, SIOCGMIIREG, &ifr) == -1)
	{
		if(sockfd >= 3)
			close(sockfd);
		return false;
	}
	if(sockfd >= 3)
		close(sockfd);
	if(((mii->val_out & BMSR_RFAULT) == 0) && ((mii->val_out & BMSR_LSTATUS) != 0))
		isUp = true;
	else
		isUp = false;
	return true;
}

bool CNetworkManager::startDhcp(const char* ethname)
{
	std::map<std::string, CDhcpClient*>::iterator it = m_dhcp_map.find(std::string(ethname));
	if(it != m_dhcp_map.end())
	{
		warnf("%s, %d: %s dhcp already started.", __FILE__, __LINE__, ethname);
		return false;
	}
	CDhcpClient* client = new CDhcpClient;
	client->startDhcp(ethname, m_hostname.c_str());
	m_dhcp_map.insert(std::make_pair<std::string, CDhcpClient*>(std::string(ethname), client));
	return true;
}

bool CNetworkManager::stopDhcp(const char* ethname)
{
	std::map<std::string, CDhcpClient*>::iterator it = m_dhcp_map.find(std::string(ethname));
	if(it == m_dhcp_map.end())
	{
		warnf("%s, %d: %s dhcp not started.", __FILE__, __LINE__, ethname);
		return false;
	}
	CDhcpClient* client = it->second;
	client->stopDhcp();
	delete client;
	m_dhcp_map.erase(it);
	return true;

}

