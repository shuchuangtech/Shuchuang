#ifndef __RPC_SERVER_H__
#define __RPC_SERVER_H__
#include "Poco/SingletonHolder.h"
#include "Poco/AbstractObserver.h"
#include "Poco/NotificationQueue.h"
#include "Poco/NotificationCenter.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Device/RequestNotification.h"
#include "Device/RPCClient.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/ThreadPool.h"
#include "Poco/Timestamp.h"
#include "Poco/Mutex.h"
using namespace Poco;
class CRPCServer : public Runnable
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
	void handleFinish(TaskFinishedNotification* pNf);
	bool addRequest(RequestNotification::Ptr rf);
	bool addObserver(const AbstractObserver& o);
	bool removeObserver(const AbstractObserver& o);
private:
	NotificationQueue	m_queue;
	NotificationCenter	m_center;
	Thread				m_thread;
	TaskManager*		m_task_manager;
	ThreadPool*			m_thread_pool;
	bool				m_started;
	Mutex				m_mutex;
};
#endif

