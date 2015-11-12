#include "Device/SystemManager.h"
#include "Common/PrintLog.h"
using namespace Poco;
CSystemManager::CSystemManager()
{
	m_started = false;
	m_network = NULL;
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
	m_network = CNetworkManager::instance();
	if(m_network == NULL)
	{
		warnf("%s, %d: Get network manager instance failed.", __FILE__, __LINE__);
		return false;
	}
	//init eth0 dhcp state false
	m_dhcp_state.insert(std::make_pair<std::string, bool>("eth0", false));
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
		bool up = false;
		m_network->getMiiLinkState("eth0", up);
		if(!up)
		{
			m_network->startDhcp("eth0");
		}
		//
		Thread::sleep(5 * 1000);
	}
}

