#ifndef __DEVICE_SESSION_MANAGER_H__
#define __DEVICE_SESSION_MANAGER_H__
#include "Poco/Types.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
using namespace Poco;
class CSessionManager : public Runnable
{
public:
	CSessionManager();
	~CSessionManager();
	bool start();
	bool stop();
	void run();
	bool connect(DynamicStruct* param);
	bool disconnect(DynamicStruct* param);
private:
	bool				m_started;
	Thread				m_thread;
	Mutex				m_mutex;
	std::map<int, CSessionClient*>	m_session_client_map;
};
#endif
