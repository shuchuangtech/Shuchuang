#ifndef __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#define __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#include "Poco/SingletonHolder.h"
class CDeviceController
{
public:
	CDeviceController();
	~CDeviceController();
	static CDeviceController* instance()
	{
		static Poco::SingletonHolder<CDeviceController> sh;
		return sh.get();
	}
	bool openDevice();
	bool isOpen();
	bool openDoor();
	bool closeDoor();
	bool setConfig();
private:
	int	m_fd;
};
#endif

