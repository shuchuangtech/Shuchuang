#include "LedController.h"
#include <stdio.h>
CLedController::CLedController()
	:m_thread("LedController")
{
	m_pfd_inner[0] = 0;
	m_pfd_inner[1] = 0;

	m_pfd_running[0] = 0;
	m_pfd_running[1] = 0;

	m_pfd_power[0] = 0;
	m_pfd_power[1] = 0;

	m_pfd_relay[0] = 0;
	m_pfd_relay[0] = 0;

	m_started = false;
}

CLedController::~CLedController()
{
}

bool CLedController::init()
{
	if(m_started)
	{
		return true;
	}
	if(pipe(m_pfd_inner) < 0)
	{
		return false;
	}
	if(pipe(m_pfd_running) < 0)
	{
		close(m_pfd_inner[0]);
		close(m_pfd_inner[1]);
		m_pfd_inner[0] = 0;
		m_pfd_inner[1] = 0;
		return false;
	}
	if(pipe(m_pfd_power) < 0)
	{
		close(m_pfd_inner[0]);
		close(m_pfd_inner[1]);
		m_pfd_inner[0] = 0;
		m_pfd_inner[1] = 0;

		close(m_pfd_running[0]);
		close(m_pfd_running[1]);
		m_pfd_running[0] = 0;
		m_pfd_running[1] = 0;
		return false;
	}
	if(pipe(m_pfd_relay) < 0)
	{
		close(m_pfd_inner[0]);
		close(m_pfd_inner[1]);
		m_pfd_inner[0] = 0;
		m_pfd_inner[1] = 0;

		close(m_pfd_running[0]);
		close(m_pfd_running[1]);
		m_pfd_running[0] = 0;
		m_pfd_running[1] = 0;

		close(m_pfd_power[0]);
		close(m_pfd_power[1]);
		m_pfd_power[0] = 0;
		m_pfd_power[1] = 0;
		return false;
	}
	printf("inner:%d, running:%d, power:%d, relay:%d\n", m_pfd_inner[0], m_pfd_running[0], m_pfd_power[0], m_pfd_relay[0]);
	FD_ZERO(m_rdfds);
	FD_SET(m_pfd_inner[0], m_rdfds);
	FD_SET(m_pfd_running[0], m_rdfds);
	FD_SET(m_pfd_power[0], m_rdfds);
	FD_SET(m_pfd_relay[0], m_rdfds);
	m_thread.start(*this);
	m_started = true;
	return true;
}

bool CLedController::closeAll()
{
	if(!m_started)
		return true;
	if(m_pfd_inner[0] != 0)
	{
		close(m_pfd_inner[0]);
		m_pfd_inner[0] = 0;
	}
	if(m_pfd_inner[1] != 0)
	{
		close(m_pfd_inner[1]);
		m_pfd_inner[1] = 0;
	}
	if(m_pfd_running[0] != 0)
	{
		close(m_pfd_running[0]);
		m_pfd_running[0] = 0;
	}
	if(m_pfd_running[1] != 0)
	{
		close(m_pfd_running[1]);
		m_pfd_running[1] = 0;
	}
	if(m_pfd_power[0] != 0)
	{
		close(m_pfd_power[0]);
		m_pfd_power[0] = 0;
	}
	if(m_pfd_power[1] != 0)
	{
		close(m_pfd_power[1]);
		m_pfd_power[1] = 0;
	}
	if(m_pfd_relay[0] != 0)
	{
		close(m_pfd_relay[0]);
		m_pfd_relay[0] = 0;
	}
	if(m_pfd_relay[1] != 0)
	{
		close(m_pfd_relay[1]);
		m_pfd_relay[1] = 0;
	}
	m_thread.join();
	m_started = false;
	return true;
}

void CLedController::run()
{
	while(m_started)
	{
		int ret = select(m_pfd_relay[0] + 1, &m_rd_fds, NULL, NULL, )
	}
}

