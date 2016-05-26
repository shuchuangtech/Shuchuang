#include "Device/Component/Task/TaskHandler.h"
#include "Common/PrintLog.h"
#include "Poco/DateTime.h"
#include "Device/Component/System/SystemManager.h"
#include "Device/Component/DeviceController.h"
using namespace Poco;
CTaskHandler::CTaskHandler()
{
	m_running = false;
	m_active = false;
}

CTaskHandler::~CTaskHandler()
{
}

void CTaskHandler::run()
{
	m_running = true;
	DateTime now;
	int dayOffset = now.dayOfWeek();
	int timeDiff = CSystemManager::instance()->getUTCOffset();
	now.makeLocal(timeDiff);
	UInt8 dayMask = TaskSunday >> dayOffset;
	if(dayMask & m_weekday)
	{
		infof("%s, %d: TaskHandler[%lld] option %d run at %02d:%02d weekday %d.", __FILE__, __LINE__, m_id, m_option, m_hour, m_minute, m_weekday);
		JSON::Object::Ptr nil = NULL;
		std::string detail;
		if(m_option == 0)
		{
			CDeviceController::instance()->closeDoor(nil, detail);
		}
		else
		{
			CDeviceController::instance()->openDoor(nil, detail);
		}
	}
	else
	{
		//nothing to do
	}
}

void CTaskHandler::setTaskInfo(TaskInfo task)
{
	if(m_running)
		return;
	m_id = task.id;
	m_option = task.option;
	m_hour = task.hour;
	m_minute = task.minute;
	m_weekday = task.weekday;
	m_active = (task.active == 1);
}

Int64 CTaskHandler::getId()
{
	return m_id;
}

int CTaskHandler::getOption()
{
	return m_option;
}

int CTaskHandler::getHour()
{
	return m_hour;
}

int CTaskHandler::getMinute()
{
	return m_minute;
}

UInt8 CTaskHandler::getWeekday()
{
	return m_weekday;
}

bool CTaskHandler::isActive()
{
	return m_active;
}

