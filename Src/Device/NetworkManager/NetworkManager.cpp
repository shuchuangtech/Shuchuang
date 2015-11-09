#include "Device/NetworkManager.h"
#include <string.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Poco/Types.h"
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
}

bool CNetworkManager::stopDhcp(const char* ethname)
{
}

