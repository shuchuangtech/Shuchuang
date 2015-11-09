#ifndef __DEVICE_COMPONENT_TASK_HANDLER_H__
#define __DEVICE_COMPONENT_TASK_HANDLER_H__
#include "Poco/Util/TimerTask.h"
#include "Device/Component/Task/TaskInfo.h"
#include "Poco/JSON/Object.h"
class CTaskHandler : public Poco::Util::TimerTask
{
public:
	typedef Poco::AutoPtr<CTaskHandler> Ptr;
	CTaskHandler();
	~CTaskHandler();
	void			run();
	void			setTaskInfo(TaskInfo task);
	Poco::UInt64	getId();
	int				getOption();
	int				getHour();
	int				getMinute();
	Poco::UInt8		getWeekday();
private:
	Poco::UInt64	m_id;
	bool			m_running;
	int				m_option;
	int				m_hour;
	int				m_minute;
	Poco::UInt8		m_weekday;
};
#endif

