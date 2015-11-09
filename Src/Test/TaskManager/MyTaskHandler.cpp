#include "MyTaskHandler.h"
#include "Common/PrintLog.h"
#include <stdio.h>
CMyTaskHandler::CMyTaskHandler()
{
	m_repeat = 0;
}

CMyTaskHandler::~CMyTaskHandler()
{
}

void CMyTaskHandler::setTaskInfo(int id)
{
	m_id = id;
}

void CMyTaskHandler::run()
{
	tracef("Task %d print %d times.", m_id, ++m_repeat);
}
