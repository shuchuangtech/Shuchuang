#include "Device/Component/DeviceController.h"
#include "Common/PrintLog.h"
#include "scgpio/gpioapi.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#define __SC_IN_NORMAL_CLOSE__
#define __SC_ON_NORMAL_CLOSE__
using namespace Poco;
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
	std::string file = "/dev/hzgpiodriver";
	m_fd = open(file.c_str(), O_RDWR);
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
	m_fd = 1;
	tracef("%s, %d: X86 does not implement openDevice.");
	return true;
#endif
}

bool CDeviceController::checkDoor(JSON::Object::Ptr& param, std::string& detail)
{
	if(m_fd == 0)
	{
		warnf("%s, %d: Maybe should call openDevice first.", __FILE__, __LINE__);
		detail = "420";
		return false;
	}
#ifdef __SC_ARM__
	int value;
	value = ioctl(m_fd, SC_READ_KEY, 0);
	if(value == 1)
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		param->set("state", "close");	
	#else
		param->set("state", "open");
	#endif
		return true;
	}
	else
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		param->set("state", "open");
	#else
		param->set("state", "close");
	#endif
		return true;
	}
#else
	tracef("%s, %d: X86 does not implement checkDoor.");
	return true;
#endif
}

bool CDeviceController::openDoor(JSON::Object::Ptr& param, std::string& detail)
{
	if(m_fd == 0)
	{
		warnf("%s, %d: Maybe should call openDevice first.", __FILE__, __LINE__);
		detail = "420";
		return false;
	}
#ifdef __SC_ARM__
#ifdef __SC_ON_NORMAL_CLOSE__
	ioctl(m_fd, SC_RELAY_ON, 0);
#else
	ioctl(m_fd, SC_RELAY_OFF, 0);
#endif
#else
	tracef("%s, %d: X86 does not implement openDoor.");
#endif
	return true;
}

bool CDeviceController::closeDoor(JSON::Object::Ptr& param, std::string& detail)
{
	if(m_fd == 0)
	{
		warnf("%s, %d: Maybe should call openDevice first.", __FILE__, __LINE__);
		detail = "420";
		return false;
	}
#ifdef __SC_ARM__
#ifdef __SC_ON_NORMAL_CLOSE__
	ioctl(m_fd, SC_RELAY_OFF, 0);
#else
	ioctl(m_fd, SC_RELAY_ON, 0);
#endif
#else
	tracef("%s, %d: X86 does not implement closeDoor.");
#endif
	return true;
}

