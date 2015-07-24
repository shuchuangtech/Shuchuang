#ifndef __DEVICE_SESSION_MANAGER_H__
#define __DEVICE_SESSION_MANAGER_H__
#include "Poco/Types.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Device/SessionClient.h"
#include "Poco/SingletonHolder.h"
using namespace Poco;
class CSessionManager : public Runnable
{
public:
	CSessionManager();
	~CSessionManager();
	CSessionManager* instance()
	{
		static SingletonHolder<CSessionManager> sh;
		return sh.get();
	};
	bool start();
	bool stop();
	void run();
	bool connect(DynamicStruct* param);
	bool disconnect(DynamicStruct* param);
private:
	bool				m_started;
	Thread				m_thread;
	Mutex				m_mutex;
	//session id(sockfd)
	std::map<int, CSessionClient*>	m_session_client_map;
};
#endif
