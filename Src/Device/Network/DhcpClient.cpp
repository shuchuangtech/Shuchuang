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
	if(readPidFile())
	{
		stopDhcp();
	}
	m_argv = new char*[10];
	for(int i = 0; i < 10; i++)
		m_argv[i] = new char[16];
	int index = 0;
	snprintf(m_argv[index++], 15, "%s", "udhcpc");
	snprintf(m_argv[index++], 15, "%s", "-i");
	snprintf(m_argv[index++], 15, "%s", ethname);
	snprintf(m_argv[index++], 15, "%s", "-p");
	snprintf(m_argv[index++], 15, "%s", "/var/run/udhcpc.pid");
	snprintf(m_argv[index++], 15, "%s", "-R");
	snprintf(m_argv[index++], 15, "%s", "-b");
	snprintf(m_argv[index++], 15, "%s", "-h");
	snprintf(m_argv[index++], 15, "%s", hostname);
	m_argv[index++] = NULL;
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
		int index = 0;
		char* const* argv = m_argv;
		char* envp[] = {NULL};
		execve("/sbin/udhcpc", argv, envp);
	}
	return true;
}

bool CDhcpClient::readPidFile()
{
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
	return true;
}

bool CDhcpClient::stopDhcp()
{
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
	return true;
}

