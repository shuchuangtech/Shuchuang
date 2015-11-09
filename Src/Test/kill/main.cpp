#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Poco/Thread.h"
bool readPidFile(int& pid)
{
	FILE* file = fopen("/var/run/udhcpc.pid", "r");
	if(file == NULL)
		return false;
	fscanf(file, "%d", &pid);
	if(kill(pid, 0) != 0)
		return false;
	return true;
}

int main(int argc, char** argv)
{
	pid_t pid = -1;
	pid = fork();
	if(pid < 0)
	{
		printf("fork error.\n");
		return -1;
	}
	else if(pid > 0)
	{
		printf("parent process, child's pid %d.\n", pid);
		int dhcp_pid;
		int status;
		if(waitpid(pid, &status, 0) < 0)
		{
			printf("waitpid error");
			return -1;
		}
		for(int i = 0; i < 5; i++)
		{
			if(readPidFile(dhcp_pid))
			{
				printf("read pid file successfully pid[%d]\n", dhcp_pid);
				break;
			}
			else
			{
				printf("process not start.\n");
			}
			Poco::Thread::sleep(1 * 1000);
		}
		Poco::Thread::sleep(5 * 1000);
		printf("before kill dhcp[%d].\n", dhcp_pid);
		if(kill(dhcp_pid, SIGKILL) == 0)
		{
			printf("dhcp process[%d] stop.\n", dhcp_pid);
		}
		Poco::Thread::sleep(10 * 1000);
	}
	else
	{
		char* argv[] = {"udhcpc", "-i", "eth0", "-b", "-R", "-p", "/var/run/udhcpc.pid", NULL};
		char* envp[] = {NULL};
		execve("/sbin/udhcpc", argv, envp);
	}

	return 0;
}

