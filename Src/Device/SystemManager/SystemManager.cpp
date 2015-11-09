#include "Device/SystemManager.h"
#include "Common/PrintLog.h"
using namespace Poco;
CSystemManager::CSystemManager()
{
	m_started = false;
}

CSystemManager::~CSystemManager()
{
}

bool CSystemManager::start()
{
	if(m_started)
	{
		warnf("%s, %d: SystemManager alread started.", __FILE__, __LINE__);
		return false;
	}
	m_started = true;
	m_thread.start(*this);
	infof("%s, %d: SystemManager start successfully.", __FILE__, __LINE__);
	return true;
}

bool CSystemManager::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: SystemManager not started.", __FILE__, __LINE__);
		return false;
	}
	m_started = false;
	infof("%s, %d: SystemManager stop successfully.", __FILE__, __LINE__);
	return true;
}

void CSystemManager::run()
{
	while(m_started)
	{
		//check interface link state
	}
}

