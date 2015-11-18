#ifndef __DEVICE_NETWORK_DHCP_CLIENT_H__
#define __DEVICE_NETWORK_DHCP_CLIENT_H__
class CDhcpClient
{
public:
	CDhcpClient();
	~CDhcpClient();
	bool	startDhcp(const char* ethname, const char* hostname);
	bool	stopDhcp();
private:
	bool			readPidFile();
	int				m_pid;
	char**			m_argv;
};
#endif

