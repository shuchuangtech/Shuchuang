#include "Device/Component/Task/TaskManager.h"
#include "Device/Component/Task/TaskHandler.h"
#include "Common/PrintLog.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/Timezone.h"
#define min(x,y) ((x) < (y) ? (x) : (y))
using namespace Poco;
CTaskManager::CTaskManager()
:m_max_task_num(32)
{
	m_timer = NULL;
	m_config = NULL;
	m_task_config = NULL;
	m_task_map.clear();
	m_config = CConfigManager::instance();
	loadTasksConfig();
}

CTaskManager::~CTaskManager()
{
	if(m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}
	m_config = NULL;
	tracef("%s, %d: Task manager clearing tasks info.", __FILE__, __LINE__);
	m_task_map.clear();
}

bool CTaskManager::stopAllTasks(bool wait)
{
	for(std::map<Int64, CTaskHandler::Ptr>::iterator it = m_task_map.begin(); it != m_task_map.end(); it++)
	{
		tracef("%s, %d: Task[%lld] canceled.", __FILE__, __LINE__, it->second->getId());
		it->second->cancel();
		it->second = NULL;
	}
	infof("%s, %d: All tasks canceled.", __FILE__, __LINE__);
	m_timer->cancel(wait);
	return true;
}

//calculate the first execute time and call timer schedule
//execute state check in TaskHandler
void CTaskManager::addToScheduleQueue(CTaskHandler::Ptr pTask)
{
	DateTime now;
	now.makeLocal(Timezone::tzd());
	int hour = pTask->getHour();
	int minute = pTask->getMinute();
	DateTime base(now.year(),
				now.month(),
				now.day(),
				now.hour(),
				now.minute());
	DateTime des(now.year(),
				now.month(),
				now.day(),
				hour, 
				minute);
	Timespan diff = des - base;
	int diffMinutes = diff.totalMinutes();
	diffMinutes = (diffMinutes + 24 * 60) % (24 * 60);
	//if diff minutes <= 1, then set the task tomorrow
	if(diffMinutes <= 1)
		diffMinutes += 24 * 60;
	DateTime executeTime = base + Timespan(0, 0, diffMinutes, 0, 0);
	infof("%s, %d: Task scheduled, execute at %04d-%02d-%02d %02d:%02d first time.", __FILE__, __LINE__, executeTime.year(), executeTime.month(), executeTime.day(), executeTime.hour(), executeTime.minute());
	executeTime.makeUTC(Timezone::tzd());
	m_timer->schedule(pTask, executeTime.timestamp(), 24 * 60 * 60 * 1000);
}

void CTaskManager::loadTasksConfig()
{
	infof("%s, %d: TaskManager loading tasks from config...", __FILE__, __LINE__);
	if(m_timer == NULL)
		m_timer = new Util::Timer;
	JSON::Array::Ptr pArray;
	m_config->getConfig("Tasks", pArray);
	m_task_config = new JSON::Array;
	if(!pArray.isNull())
	{
		m_mutex.lock();
		for(unsigned int i = 0; i < pArray->size(); i++)
		{
			TaskInfo t;
			DynamicStruct ds = (DynamicStruct)(*(pArray->getObject(i)));
			m_task_config->add(ds);
			structToTaskInfo(ds, t);
			CTaskHandler::Ptr pTask = new CTaskHandler;
			pTask->setTaskInfo(t);
			addToScheduleQueue(pTask);
			m_task_map.insert(std::make_pair<Int64, CTaskHandler::Ptr>(t.id, pTask));
			infof("%s, %d: Task loaded[id:%lld, option:%d, hour:%d, minute:%d, weekday:%d].", __FILE__, __LINE__, t.id, t.option, t.hour, t.minute, t.weekday);
		}
		m_mutex.unlock();
	}
}

int CTaskManager::getTasksNumber()
{
	m_mutex.lock();
	int ret = m_task_map.size();
	m_mutex.unlock();
	return ret;
}

bool CTaskManager::getTasks(JSON::Object::Ptr& param, std::string& detail)
{
	m_mutex.lock();
	//can't use da.size, weired
	JSON::Array::Ptr array = new JSON::Array;
	for(unsigned int i = 0; i < m_task_config->size(); i++)
	{
		Dynamic::Var var = m_task_config->get(i);
		DynamicStruct ds = var.extract<DynamicStruct>();
		array->add(ds);
	}
	param->set("tasks", array);
	if(param->has("token"))
	{
		param->remove("token");
	}
	m_mutex.unlock();
	return true;
}

void CTaskManager::taskInfoToStruct(const TaskInfo& task, DynamicStruct& ds)
{
	ds["id"] = task.id;
	ds["option"] = task.option;
	ds["hour"]= task.hour;
	ds["minute"] = task.minute;
	ds["weekday"] = task.weekday;
}

void CTaskManager::structToTaskInfo(const DynamicStruct& ds, TaskInfo& task)
{
	Dynamic::Var var;
	var = ds["id"];
	task.id = var.extract<Int64>();
	var = ds["option"];
	task.option = var.extract<int>();
	var = ds["hour"];
	task.hour = var.extract<int>();
	var = ds["minute"];
	task.minute = var.extract<int>();
	var = ds["weekday"];
	task.weekday = var.extract<int>();
}

bool CTaskManager::taskExists(const TaskInfo& task)
{
	Mutex::ScopedLock lock(m_mutex);
	for(std::map<Int64, CTaskHandler::Ptr>::iterator it = m_task_map.begin(); it != m_task_map.end(); it++)
	{
		if(it->second->getHour() == task.hour && it->second->getMinute() == task.minute)
			return true;
	}
	return false;
}

bool CTaskManager::addTask(JSON::Object::Ptr& param, std::string& detail)
{
	if(m_task_map.size() == m_max_task_num)
	{
		detail = "430";
		return false;
	}
	if(param.isNull() || !param->has("task"))
	{
		detail = "431";
		return false;
	}
	JSON::Object::Ptr pObjTask = param->getObject("task");
	if(pObjTask.isNull() || !pObjTask->has("option") || !pObjTask->has("hour")
		|| !pObjTask->has("minute") || !pObjTask->has("weekday"))
	{
		detail = "432";
		return false;
	}
	DynamicStruct dsParam = *pObjTask;
	//use timestamp as id
	Timestamp t;
	dsParam["id"] = (Int64)t.epochMicroseconds();
	TaskInfo task;
	structToTaskInfo(dsParam, task);
	if((task.option != 0 && task.option != 1) ||
		(task.hour < 0 || task.hour > 23) ||
		(task.minute < 0 || task.minute > 59) ||
		(task.weekday < 0 || task.weekday > 0x7F))
	{
		detail = "433";
		return false;
	}
	if(taskExists(task))
	{
		detail = "434";
		return false;
	}
	m_mutex.lock();
	//update configs
	DynamicStruct ds;
	taskInfoToStruct(task, ds);
	m_task_config->add(ds);
	m_config->setConfig("Tasks", m_task_config);
	CTaskHandler::Ptr pTask = new CTaskHandler;
	pTask->setTaskInfo(task);
	m_task_map.insert(std::make_pair<Int64, CTaskHandler::Ptr>(task.id, pTask));
	addToScheduleQueue(pTask);
	m_mutex.unlock();
	pObjTask->set("id", task.id);
	param->set("task", pObjTask);
	if(param->has("token"))
	{
		param->remove("token");
	}
	infof("%s, %d: Task added[id:%lld, option:%d, hour:%d, minute:%d, weekday:%d].", __FILE__, __LINE__, task.id, task.option, task.hour, task.minute, task.weekday);
	return true;
}

bool CTaskManager::removeTask(JSON::Object::Ptr& param, std::string& detail)
{
	if(param.isNull() || !param->has("task"))
	{
		detail = "431";
		return false;
	}
	JSON::Object::Ptr pObjTask = param->getObject("task");
	if(pObjTask.isNull() || !pObjTask->has("id"))
	{
		detail = "432";
		return false;
	}
	Int64 id = pObjTask->getValue<Int64>("id");
	m_mutex.lock();
	std::map<Int64, CTaskHandler::Ptr>::iterator it = m_task_map.find(id);
	if(it == m_task_map.end())
	{
		m_mutex.unlock();
		detail = "435";
		return false;
	}
	CTaskHandler::Ptr pTask = it->second;
	pTask->cancel();
	infof("Task (%lld, %d, %d, %d, %d) canceled.", id, it->second->getOption(), it->second->getHour(), it->second->getMinute(), it->second->getWeekday());
	m_task_map.erase(it);
	for(unsigned int i = 0; i < m_task_config->size(); i++)
	{
		Dynamic::Var var = m_task_config->get(i);
		DynamicStruct ds = var.extract<DynamicStruct>();
		Int64 taskConfig_id = (ds["id"].extract<Int64>());
		if(taskConfig_id == id)
		{
			m_task_config->remove(i);
			m_config->setConfig("Tasks", m_task_config);
			tracef("Task (%lld, %d, %d, %d, %d) deleted.", taskConfig_id,
					ds["option"].extract<int>(), ds["hour"].extract<int>(),
					ds["minute"].extract<int>(), ds["weekday"].extract<int>());
			break;
		}
	}
	if(param->has("token"))
	{
		param->remove("token");
	}
	m_mutex.unlock();
	return true;
}

bool CTaskManager::modifyTask(JSON::Object::Ptr& param, std::string& detail)
{
	if(param.isNull() || !param->has("task"))
	{
		detail = "431";
		return false;
	}
	JSON::Object::Ptr pObjTask = param->getObject("task");
	if(pObjTask.isNull() || !pObjTask->has("id") || !pObjTask->has("option") || !pObjTask->has("hour") || !pObjTask->has("minute") || !pObjTask->has("weekday"))
	{
		detail = "432";
		return false;
	}
	DynamicStruct dsParam = *pObjTask;
	TaskInfo task;
	structToTaskInfo(dsParam, task);
	Int64 id = task.id;
	infof("%s, %d: Task[id: %lld, option: %d, hour: %d, minute: %d, weekday: %d] to be changed.", __FILE__, __LINE__, task.id, task.option, task.hour, task.minute, task.weekday);
	m_mutex.lock();
	std::map<Int64, CTaskHandler::Ptr>::iterator it = m_task_map.find(id);
	if(it == m_task_map.end())
	{
		m_mutex.unlock();
		detail = "435";
		return false;
	}
	if((task.option != 0 && task.option != 1)
		|| (task.hour < 0 || task.hour > 23)
		|| (task.minute < 0 || task.minute > 59)
		|| (task.weekday < 0 || task.weekday > 0x7F))
	{
		m_mutex.unlock();
		detail = "432";
		return false;
	}
	if(taskExists(task))
	{
		m_mutex.unlock();
		detail = "434";
		return false;
	}
	//adjust TaskTimer and TaskHandler map
	infof("%s, %d: old task[%lld](%d, %d, %d, %d) stopped.", __FILE__, __LINE__, it->second->getId(), it->second->getOption(), it->second->getHour(), it->second->getMinute(), it->second->getWeekday());
	it->second->cancel();
	m_task_map.erase(it);
	CTaskHandler::Ptr pTask = new CTaskHandler;
	pTask->setTaskInfo(task);
	m_task_map.insert(std::make_pair<Int64, CTaskHandler::Ptr>(task.id, pTask));
	addToScheduleQueue(pTask);
	//adjust task config
	DynamicStruct ds;
	taskInfoToStruct(task, ds);
	for(unsigned int i = 0; i < m_task_config->size(); i++)
	{
		Dynamic::Var var = m_task_config->get(i);
		DynamicStruct oldds = var.extract<DynamicStruct>();
		var = oldds["id"];
		if(var.extract<Int64>() == id)
		{
			m_task_config->set(i, ds);
			m_config->setConfig("Tasks", m_task_config);
			infof("%s, %d: Task[%lld] modified (%d, %d, %d, %d).", __FILE__, __LINE__, task.id, task.option, task.hour, task.minute, task.weekday);
			break;
		}
	}
	if(param->has("token"))
	{
		param->remove("token");
	}
	m_mutex.unlock();
	return true;
}

