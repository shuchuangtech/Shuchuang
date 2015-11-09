#include "Device/Component/DeviceController.h"
#include "scgpio/gpioapi.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#define __SC_IN_NORMAL_CLOSE__
#define __SC_ON_NORMAL_CLOSE__
CDeviceController::CDeviceController()
{
	m_fd = 0;
}

CDeviceController::~CDeviceController()
{
}

bool CDeviceController::openDevice()
{
	if(m_fd != 0)
	{
		close(m_fd);
		m_fd = 0;
	}
#ifdef __SC_ARM__
	char* file = "/dev/hzgpiodriver";
	m_fd = open(file, O_RDWR);
	if(m_fd == -1)
		return false;
	else
		return true;
	#ifdef __SC_ON_NORMAL_CLOSE__
	ioctl(m_fd, SC_RELAY_OFF, 0);
	#else
	ioctl(m_fd, SC_RELAY_ON, 0);
	#endif
#else
	return true;
#endif
}

bool CDeviceController::isOpen()
{
	if(m_fd == 0)
		return false;
	int value;
	value = ioctl(m_fd, SC_READ_KEY, 0);
	if(value == 1)
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		return false;
	#else
		return true;
	#endif
	}
	else
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		return true;
	#else
		return false;
	#endif
	}
}

bool CDeviceController::openDoor()
{
	if(m_fd == 0)
		return false;
#ifdef __SC_ON_NORMAL_CLOSE__
	ioctl(m_fd, SC_RELAY_ON, 0);
#else
	ioctl(m_fd, SC_RELAY_OFF, 0);
#endif
	return true;
}

bool CDeviceController::closeDoor()
{
	if(m_fd == 0)
		return false;
#ifdef __SC_ON_NORMAL_CLOSE__
	ioctl(m_fd, SC_RELAY_OFF, 0);
#else
	ioctl(m_fd, SC_RELAY_ON, 0);
#endif
	return true;
}

bool CDeviceController::setConfig()
{
#ifdef __SC_ARM__
	return true;
#else
	return true;
#endif
}

