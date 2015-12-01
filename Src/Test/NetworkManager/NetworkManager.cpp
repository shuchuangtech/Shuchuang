#include "Device/Network/NetworkManager.h"
#include "Common/ConfigManager.h"
#include <string.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Common/PrintLog.h"
#include "Poco/Types.h"
#include "Poco/JSON/Object.h"
CNetworkManager::CNetworkManager()
{
}

CNetworkManager::~CNetworkManager()
{
}

bool CNetworkManager::setIfDown(const char* ethname)
{
	struct ifreq ifr;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		warnf("%s, %d: Socket open failed.", __FILE__, __LINE__);
		return false;
	}
	strncpy(ifr.ifr_name, ethname, IFNAMSIZ - 1);
	if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		warnf("%s, %d: ioctl SIOCGIFFLAGS failed.", __FILE__, __LINE__);
		close(sockfd);
		return false;
	}
	ifr.ifr_flags &= ~IFF_UP;
	if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
	{
		warnf("%s, %d: ioctl SIOCSIFFLAGS failed.", __FILE__, __LINE__);
		close(sockfd);
		return false;
	}
	infof("%s, %d: Interface %s set down.", __FILE__, __LINE__, ethname);
	close(sockfd);
	return true;
}

bool CNetworkManager::setIfUp(const char* ethname)
{
	struct ifreq ifr;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		warnf("%s, %d: Socket open failed.", __FILE__, __LINE__);
		return false;
	}
	strncpy(ifr.ifr_name, ethname, IFNAMSIZ - 1);
	if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		warnf("%s, %d: ioctl SIOCGIFFLAGS failed.", __FILE__, __LINE__);
		close(sockfd);
		return false;
	}
	ifr.ifr_flags |= IFF_UP;
	if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
	{
		warnf("%s, %d: ioctl SIOCSIFFLAGS failed.", __FILE__, __LINE__);
		close(sockfd);
		return false;
	}
	infof("%s, %d: Interface %s set up.", __FILE__, __LINE__, ethname);
	close(sockfd);
	return true;
}

bool CNetworkManager::setIfMac(const char* ethname, const char* mac)
{
	struct ifreq ifr;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		warnf("%s, %d: Socket open failed.", __FILE__, __LINE__);
		return false;
	}
	ifr.ifr_addr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, ethname, IFNAMSIZ - 1);
	memcpy((unsigned char*)ifr.ifr_hwaddr.sa_data, mac, 6);
	if(ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0)
	{
		warnf("%s, %d: ioctl SIOSIFHWADDR failed.", __FILE__, __LINE__);
		close(sockfd);
		return false;
	}
	infof("%s, %d: Interface %s set mac address[%02x:%02x:%02x:%02x:%02x:%02x].", __FILE__, __LINE__, ethname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	close(sockfd);
	return true;
}

bool CNetworkManager::initIf(const char* ethname)
{
	CConfigManager* config = CConfigManager::instance();
	Poco::JSON::Object::Ptr pConfig = NULL;
	config->getConfig("DeviceInfo", pConfig);
	if(pConfig.isNull() || !pConfig->has("mac"))
	{
		warnf("%s, %d: Get Mac address config failed.", __FILE__, __LINE__);
		return false;
	}
	std::string mac_str = pConfig->getValue<std::string>("mac");
	if(mac_str.length() < 12)
	{
		warnf("%s, %d: Mac address length error.", __FILE__, __LINE__);
		return false;
	}
	const char* temp = mac_str.c_str();
	char mac[6] = {0, };
	for(int i = 0; i < 12; i++)
	{
		char t = 0;
		if(temp[i] <= '9' && temp[i] >= '0')
		{
			t = temp[i] - '0';
		}
		else if(temp[i] <= 'f' && temp[i] >= 'a')
		{
			t = temp[i] - 'a' + 10;
		}
		else if(temp[i] <= 'F' && temp[i] >= 'A')
		{
			t = temp[i] - 'A' + 10;
		}
		else
		{
			warnf("%s, %d: Mac address format error.", __FILE__, __LINE__);
			return false;
		}
		int index = i / 2;
		int hi = i % 2;
		mac[index] += (hi == 0 ? t << 4 : t);
	}
	bool ret = true;
	ret = ret && setIfDown("eth0");
	tracepoint();
	ret = ret && setIfMac("eth0", mac);
	tracepoint();
	ret = ret && setIfUp("eth0");
	tracepoint();
	return ret;
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
	CConfigManager* config = CConfigManager::instance();
	Poco::JSON::Object::Ptr pConfig;
	config->getConfig("DeviceInfo", pConfig);
	std::string hostname = pConfig->getValue<std::string>("uuid");
	CDhcpClient* client = new CDhcpClient;
	client->startDhcp(ethname, hostname.c_str());
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

