#ifndef __LED_CONTROLLER_H__
#define __LED_CONTROLLER_H__
#include "Poco/SingletonHolder.h"
#include <unistd.h>
#include <sys/select.h>
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
class CLedController : Poco::Runnable
{
public:
	static CLedController * instance()
	{
		static Poco::SingletonHolder<CLedController> sh;
		return sh.get();
	}
	CLedController();
	~CLedController();
	bool init();
	void run();
	bool closeAll();
	bool setLedRunning(bool on);
	bool setLedPower(bool on);
	bool setLedRelay(bool on);
private:
	Poco::Thread	m_thread;
	bool		m_started;
	fd_set		m_rdfds;
	int			m_pfd_inner[2];
	int			m_pfd_running[2];
	int			m_pfd_power[2];
	int			m_pfd_relay[2];
};
#endif

