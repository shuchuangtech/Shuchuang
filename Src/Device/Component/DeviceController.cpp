#include "Device/Component/DeviceController.h"
#include "Poco/Timestamp.h"
#include "Poco/Timezone.h"
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
	m_user_record = CUserRecord::instance();
	m_op_record = COperationRecord::instance();
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
		#else
		ioctl(m_fd, SC_RELAY_ON, 0);
		#endif
		return true;
	}
#else
	m_fd = 1;
	tracef("%s, %d: X86 does not implement openDevice.", __FILE__, __LINE__);
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
	}
	else
	{
	#ifdef __SC_IN_NORMAL_CLOSE__
		param->set("state", "open");
	#else
		param->set("state", "close");
	#endif
	}
	infof("%s, %d: Check door state:%s.", __FILE__, __LINE__, param->getValue<std::string>("state").c_str());
#else
	tracef("%s, %d: X86 does not implement checkDoor.", __FILE__, __LINE__);
#endif
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
	m_op_record->addRecord(op);
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
	m_op_record->addRecord(op);
	return true;
}

