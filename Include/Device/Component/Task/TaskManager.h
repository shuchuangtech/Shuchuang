#ifndef __DEVICE_COMPONENT_TASK_MANAGER_H__
#define __DEVICE_COMPONENT_TASK_MANAGER_H__
#include "Poco/Util/Timer.h"
#include "Poco/SingletonHolder.h"
#include "Poco/Mutex.h"
#include "Poco/Types.h"
#include <list>
#include "Common/ConfigManager.h"
#include "Device/Component/Task/TaskInfo.h"
#include "Device/Component/Task/TaskHandler.h"
class CTaskManager
{
public:
	CTaskManager();
	~CTaskManager();
	static CTaskManager* instance()
	{
		static Poco::SingletonHolder<CTaskManager> sh;
		return sh.get();
	}
	bool stopAllTasks(bool wait = false);
	int getTasksNumber();
	bool getTasks(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool addTask(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool removeTask(Poco::JSON::Object::Ptr& param, std::string& detail);
	bool modifyTask(Poco::JSON::Object::Ptr& param, std::string& detail);
private:
	void addToScheduleQueue(CTaskHandler::Ptr pTask);
	bool taskExists(const TaskInfo& task);
	void loadTasksConfig();
	void taskInfoToStruct(const TaskInfo&, Poco::DynamicStruct&);
	void structToTaskInfo(const Poco::DynamicStruct&, TaskInfo&);
	Poco::Util::Timer*						m_timer;
	CConfigManager*							m_config;
	Poco::JSON::Array::Ptr					m_task_config;
	const unsigned int						m_max_task_num;
	std::map<Poco::Int64, CTaskHandler::Ptr>		m_task_map;
	Poco::Mutex								m_mutex;
};
#endif

