#ifndef __RPC_SERVER_H__
#define __RPC_SERVER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/AbstractObserver.h"
#include "Poco/NotificationQueue.h"
#include "Poco/NotificationCenter.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Device/Notification/RequestNotification.h"
#include "Device/RPC/RPCClient.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/Mutex.h"
class CRPCServer : public Poco::Runnable
{
public:
	CRPCServer();
	~CRPCServer();
	static CRPCServer* instance()
	{
		static Poco::SingletonHolder<CRPCServer> sh;
		return sh.get();
	}
	void start();
	void stop();
	void run();
	void handleFinish(Poco::TaskFinishedNotification* pNf);
	bool addRequest(RequestNotification::Ptr rf);
	bool addObserver(const Poco::AbstractObserver& o);
	bool removeObserver(const Poco::AbstractObserver& o);
private:
	Poco::NotificationQueue		m_queue;
	Poco::NotificationCenter	m_center;
	Poco::Thread				m_thread;
	Poco::TaskManager*			m_task_manager;
	Poco::ThreadPool*			m_thread_pool;
	bool						m_started;
	Poco::Mutex					m_mutex;
};
#endif

