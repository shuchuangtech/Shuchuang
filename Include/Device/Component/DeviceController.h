#ifndef __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#define __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Device/Component/Record/OperationManager.h"
#include "Device/Component/User/UserManager.h"
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
	//open describe file
	bool openDevice();
	//check door state
	bool checkDoor(Poco::JSON::Object::Ptr& param, std::string& detail);
	//open door
	bool openDoor(Poco::JSON::Object::Ptr& param, std::string& detail);
	//close door
	bool closeDoor(Poco::JSON::Object::Ptr& param, std::string& detail);
	//change user mode
	bool changeMode(Poco::JSON::Object::Ptr& param, std::string& detail);
	//get user mode
	bool getMode(Poco::JSON::Object::Ptr& param, std::string& detail);
	//used inside programme
	//error led
	bool errOn();
	bool errOff();
private:
	int					m_fd;
	int					m_user_mode;
	bool				m_door_open;
	CUserManager*		m_user_manager;
	COpManager*			m_op_manager;
};
#endif

