#include "MyTask.h"
#include "Common/PrintLog.h"
MyTask::MyTask()
{
	m_id = 0;
	m_repeat = 0;
}

MyTask::~MyTask()
{
}

void MyTask::setId(int id)
{
	m_id = id;
}

void MyTask::run()
{
	tracef("%s, %d: TimerTask[%d] run %d times.", __FILE__, __LINE__, m_id, ++m_repeat);
}

