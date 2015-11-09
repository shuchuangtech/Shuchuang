#include "Device/Component/Task/TaskHandler.h"
#include "Common/PrintLog.h"
using namespace Poco;
CTaskHandler::CTaskHandler()
{
	m_running = false;
}

CTaskHandler::~CTaskHandler()
{
}

void CTaskHandler::run()
{
	m_running = true;
	infof("%s, %d: TaskHandler[%llu] option %d run at %02d:%02d weekday %d.", __FILE__, __LINE__, m_id, m_option, m_hour, m_minute, m_weekday);
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
}

UInt64 CTaskHandler::getId()
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
