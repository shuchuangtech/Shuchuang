#ifndef __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#define __DEVICE_COMPONENT_DEVICE_CONTROLLER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Device/Util/UserRecord.h"
#include "Device/Component/Record/OperationManager.h"
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
private:
	int	m_fd;
	CUserRecord*		m_user_record;
	COpManager*			m_op_manager;
};
#endif

