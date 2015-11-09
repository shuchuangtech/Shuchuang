#ifndef __DEVICE_NETWORK_MANAGER_H__
#define __DEVICE_NETWORK_MANAGER_H__
class CNetworkManager
{
public:
	static bool getMiiLinkState(const char* ifname, bool& isUp);
	bool startDhcp(const char* ethname);
	bool stopDhcp(const char* ethname);
private:

};
#endif

