#ifndef __DEVICE_NETWORK_DHCP_CLIENT_H__
#define __DEVICE_NETWORK_DHCP_CLIENT_H__
class CDhcpClient
{
public:
	CDhcpClient();
	~CDhcpClient();
	bool	startDhcp(const char* ethname);
	bool	stopDhcp();
private:
	bool	readPidFile();
	int		m_pid;
};
#endif

