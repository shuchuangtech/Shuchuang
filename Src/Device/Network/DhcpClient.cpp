#include "Device/Network/DhcpClient.h"
#include "Common/PrintLog.h"
#include "Poco/Thread.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
CDhcpClient::CDhcpClient()
{
	m_argv = NULL;
	m_pid = 0;
}

CDhcpClient::~CDhcpClient()
{
	if(m_argv != NULL)
		delete[] m_argv;
	stopDhcp();
}

bool CDhcpClient::startDhcp(const char* ethname, const char* hostname)
{
#ifdef __SC_ARM__
	if(readPidFile())
	{
		stopDhcp();
	}
	m_argv = new char*[10];
	for(int i = 0; i < 10; i++)
		m_argv[i] = new char[32];
	int index = 0;
	infof("%s, %d:Start Dhcp...", __FILE__, __LINE__);
	snprintf(m_argv[index++], 31, "%s", "udhcpc");
	snprintf(m_argv[index++], 31, "%s", "-i");
	snprintf(m_argv[index++], 31, "%s", ethname);
	snprintf(m_argv[index++], 31, "%s", "-p");
	snprintf(m_argv[index++], 31, "%s", "/var/run/udhcpc.pid");
	snprintf(m_argv[index++], 31, "%s", "-R");
	snprintf(m_argv[index++], 31, "%s", "-b");
	snprintf(m_argv[index++], 31, "%s", "-h");
	snprintf(m_argv[index++], 31, "%s", hostname);
	m_argv[index++] = NULL;
	infof("%s, %d: Dhcp start argument: %s %s %s %s %s %s %s %s %s", __FILE__, __LINE__, m_argv[0], m_argv[1], m_argv[2], m_argv[3], m_argv[4], m_argv[5], m_argv[6], m_argv[7], m_argv[8]);
	pid_t pid = -1;
	pid = fork();
	if(pid < 0)
	{
		warnf("%s, %d: Error in fork.", __FILE__, __LINE__);
		return false;
	}
	else if(pid > 0)
	{
		if(waitpid(pid, NULL, 0) < 0)
		{
			errorf("%s, %d: wait children process error.", __FILE__, __LINE__);
			return false;
		}
		for(int i = 0; i < 10; i++)
			delete[] m_argv[i];
		delete[] m_argv;
		m_argv = NULL;
		//parent
		for(int i = 0; i < 5; i++)
		{
			if(readPidFile())
				break;
			Poco::Thread::sleep(1000);
		}
		if(m_pid == 0)
		{
			warnf("%s, %d: Dhcp client start failed.", __FILE__, __LINE__);
			return false;
		}
		else
		{
			infof("%s, %d: Dhcp client[%d] start successfully.", __FILE__, __LINE__, m_pid);
			return true;
		}
	}
	else
	{
		//children
		char* const* argv = m_argv;
		char* envp[] = {NULL};
		execve("/sbin/udhcpc", argv, envp);
	}
#else
	tracef("%s, %d: X86 does not implement start dhcp.", __FILE__, __LINE__);
#endif
	return true;
}

bool CDhcpClient::readPidFile()
{
#ifdef __SC_ARM__
	FILE* file = fopen("/var/run/udhcpc.pid", "r");
	if(file == NULL)
		return false;
	fscanf(file, "%d", &m_pid);
	if(kill(m_pid, 0) != 0)
	{
		warnf("%s, %d: Dhcp client process[%d] not exists.", __FILE__, __LINE__, m_pid);
		return false;
	}
	fclose(file);
	file = NULL;
#endif
	return true;
}

bool CDhcpClient::stopDhcp()
{
#ifdef __SC_ARM__
	if(m_pid == 0)
		return false;
	int retval = kill(m_pid, SIGKILL);
	if(retval == 0)
	{
		infof("%s, %d: Dhcp client[%d] stop successfully.", __FILE__, __LINE__, m_pid);
		return true;
	}
	else
	{
		warnf("%s, %d: Dhcp client[%d] stop failed.", __FILE__, __LINE__, m_pid);
		return false;
	}
	m_pid = 0;
#endif
	return true;
}

