#include "MyTaskManager.h"
#include "Poco/DateTime.h"
#include "Poco/Timespan.h"
#include <stdio.h>
using namespace Poco;
CMyTaskManager::CMyTaskManager()
{
	m_timer = NULL;
	m_timer = new Poco::Util::Timer;
}

CMyTaskManager::~CMyTaskManager()
{
	if(m_timer != NULL)
		delete m_timer;
}

void CMyTaskManager::addTask(int task_id)
{
	std::map<int, CMyTaskHandler::Ptr>::iterator it = m_map.find(task_id);
	if(it != m_map.end())
	{
		printf("task %d already exists.\n", task_id);
		return;
	}
	CMyTaskHandler::Ptr pTask = new CMyTaskHandler;
	pTask->setTaskInfo(task_id);
	m_map.insert(std::make_pair<int, CMyTaskHandler::Ptr>(task_id, pTask));
	DateTime dt;
	dt += Timespan(10, 0);
	m_timer->schedule(pTask, dt.timestamp(), 10 * 1000);
}

void CMyTaskManager::removeTask(int task_id)
{
	std::map<int, CMyTaskHandler::Ptr>::iterator it = m_map.find(task_id);
	if(it == m_map.end())
	{
		printf("task %d not exists.\n", task_id);
		return;
	}
	it->second->cancel();
	m_map.erase(it);
}

