#include "Device/Component/DeviceController.h"
#include "Common/RPCDef.h"
#include "Poco/Timestamp.h"
#include "Poco/Timezone.h"
#include "Common/PrintLog.h"
#include "scgpio/gpioapi.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
using namespace Poco;
CDeviceController::CDeviceController()
{
	m_fd = 0;
	m_door_open = false;
	m_user_record = CUserRecord::instance();
	m_op_manager = COpManager::instance();
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
	infof("%s, %d: Openning IO dev file...", __FILE__, __LINE__);
#ifdef __SC_ARM__
	std::string file = "/dev/hzgpiodriver";
	m_fd = open(file.c_str(), O_RDWR);
	if(m_fd == -1)
	{
		warnf("%s, %d: IO dev file opened failed.", __FILE__, __LINE__);
		return false;
	}
	else
	{
		infof("%s, %d: IO dev file opened successfully.", __FILE__, __LINE__);
		#ifdef __SC_ON_NORMAL_CLOSE__
		ioctl(m_fd, SC_RELAY_OFF, 0);
		ioctl(m_fd,	SC_RUN_ON, 0 );
		#else
		ioctl(m_fd, SC_RELAY_ON, 0);
		ioctl(m_fd, SC_RUN_OFF, 0);
		#endif
		m_door_open = false;
		return true;
	}
#else
	m_fd = 1;
	infof("%s, %d: X86 does not implement openDevice.", __FILE__, __LINE__);
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
	tracef("%s, %d: Check door read key value: %d", __FILE__, __FILE__, value);
	
	if(value == 1)
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		param->set(DEVICE_STATE_STR, DEVICE_CLOSE_STR);	
	#else
		param->set(DEVICE_STATE_STR, DEVICE_OPEN_STR);
	#endif
	}
	else
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		param->set(DEVICE_STATE_STR, DEVICE_OPEN_STR);
	#else
		param->set(DEVICE_STATE_STR, DEVICE_CLOSE_STR);
	#endif
	}
	
	if(m_door_open)
	{
		param->set(DEVICE_SWITCH_STR, DEVICE_OPEN_STR);
	}
	else
	{
		param->set(DEVICE_SWITCH_STR, DEVICE_CLOSE_STR);
	}
	infof("%s, %d: Check door state:%s, switch state:%s.", __FILE__, __LINE__, param->getValue<std::string>("state").c_str(), m_door_open?"open":"close");
#else
	tracef("%s, %d: X86 does not implement checkDoor.", __FILE__, __LINE__);
#endif
	if(param->has(USER_TOKEN_STR))
	{
		param->remove(USER_TOKEN_STR);
	}
	return true;
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
	tracef("%s, %d: X86 does not implement openDoor.", __FILE__, __LINE__);
#endif
	m_door_open = true;
	OperationRecordNode op = {0, 0, "", 0};
	DateTime now;
	now.makeLocal(Timezone::tzd());
	op.timestamp = now.timestamp().epochMicroseconds();
	op.operation = 1;
	op.username = "";
	op.schema = -1;
	if(param.isNull())
		//scheduled
	{
		op.schema = 1;
		infof("%s, %d: Door opened by schedule.", __FILE__, __LINE__);
	}
	else
		//manual
	{
		op.schema = 0;
		UserRecordNode user = {"", "", 0, 0, 0, "", 0, 0};
		user.token = param->getValue<std::string>("token");
		if(m_user_record->getUserByToken(user) > 0)
		{
			op.username = user.username;
		}
		infof("%s, %d: Door opened by manual[User:%s].", __FILE__, __LINE__, op.username.c_str());
	}
	m_op_manager->addRecord(op);
	if(param->has("token"))
	{
		param->remove("token");
	}
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
	tracef("%s, %d: X86 does not implement closeDoor.", __FILE__, __LINE__);
#endif
	m_door_open = false;
	OperationRecordNode op = {0, 0, "", 0};
	DateTime now;
	now.makeLocal(Timezone::tzd());
	op.timestamp = now.timestamp().epochMicroseconds();
	op.operation = 0;
	op.username = "";
	op.schema = -1;
	if(param.isNull())
		//scheduled
	{
		op.schema = 1;
		infof("%s, %d: Door closed by schedule.", __FILE__, __LINE__);
	}
	else
		//manual
	{
		op.schema = 0;
		UserRecordNode user = {"", "", 0, 0, 0, "", 0, 0};
		user.token = param->getValue<std::string>("token");
		if(m_user_record->getUserByToken(user) > 0)
		{
			op.username = user.username;
		}
		infof("%s, %d: Door closed by manual[User:%s].", __FILE__, __LINE__, op.username.c_str());
	}
	m_op_manager->addRecord(op);
	if(param->has("token"))
	{
		param->remove("token");
	}
	return true;
}

bool CDeviceController::errOn()
{
	if(m_fd == 0)
	{
		warnf("%s, %d: Maybe should call openDevice first.", __FILE__, __LINE__);
		return false;
	}
#ifdef __SC_ARM__
	ioctl(m_fd, SC_ERROR_ON, 0);
#else
	tracef("%s, %d: X86 does not implement errOn.", __FILE__, __LINE__);
#endif
	return true;
}

bool CDeviceController::errOff()
{
	if(m_fd == 0)
	{
		warnf("%s, %d: Maybe should call openDevice first.", __FILE__, __LINE__);
		return false;
	}
#ifdef __SC_ARM__
	ioctl(m_fd, SC_ERROR_OFF, 0);
#else
	tracef("%s, %d: X86 does not implement errOff.", __FILE__, __LINE__);
#endif
	return true;
}

