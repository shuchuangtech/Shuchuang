#include "Device/Component/DeviceController.h"
#include "Poco/Thread.h"
int main()
{
	CDeviceController* dev = CDeviceController::instance();
	dev->openDevice();
	Poco::Thread::sleep(2000);
	dev->openDoor();
	Poco::Thread::sleep(2000);
	dev->closeDoor();
	Poco::Thread::sleep(2000);
	return 0;
}

