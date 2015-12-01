#include "scgpio/gpioapi.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string>
#include <stdio.h>
int main()
{
	std::string file = "/dev/hzgpiodriver";
	int fd = open(file.c_str(), O_RDWR);
	ioctl(fd, SC_RUN_ON, 0);
	getchar();
	ioctl(fd, SC_RUN_OFF, 0);
	getchar();
	close(fd);
	return 0;
}

