#include "Device/RPCServer.h"
#include "Common/RPCDef.h"
#include "Poco/Thread.h"
#include "Common/PrintLog.h"
#include "Poco/Observer.h"
#include "Poco/Dynamic/Struct.h"
using namespace Poco;
CRPCServer::CRPCServer()
:m_thread("RPCServerThread")
{
	m_thread_pool = new ThreadPool("RPCThreadPool");
	m_task_manager = new TaskManager(*m_thread_pool);
	m_started = false;
}

CRPCServer::~CRPCServer()
{
	if(m_task_manager != NULL)
	{
		m_task_manager->joinAll();
		delete m_task_manager;
	}
	if(m_thread_pool != NULL)
	{
		m_thread_pool->joinAll();
		delete m_thread_pool;
	}
}

void CRPCServer::start()
{
	if(m_started)
	{
		warnf("%s, %d: RPCServer is already started.\n", __FILE__, __LINE__);
	}
	else
	{
		m_thread.start(*this);
		Observer<CRPCServer, TaskFinishedNotification> observer(*this, &CRPCServer::handleFinish);
		m_task_manager->addObserver(observer);
		m_started = true;
	}
}

void CRPCServer::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: RPCServer is not started.\n", __FILE__, __LINE__);
	}
	else
	{		
		m_queue.clear();
		m_queue.wakeUpAll();
		m_thread_pool->stopAll();
		m_thread.join();
	}
}

bool CRPCServer::addRequest(RequestNotification::Ptr rf)
{
	//DynamicStruct param = *(rf->getParam());
	//rf->setParam(param);
	m_queue.enqueueNotification(rf);
	return true;
}

/*
UInt64 CRPCServer::checkSessionID(UInt64& session_id, const UInt64& timestamp)
{	
	FastMutex::ScopedLock lock(m_mutex);
	std::map<UInt64, Timestamp>::iterator it_map;
	it_map = m_active_session.find(session_id);
	if(it_map == m_active_session.end())
	{
		std::list<UInt64>::iterator it;
		if(m_idle_session.empty())
		{
			session_id = m_session_id++;
		}
		else
		{
			session_id = m_idle_session.front();
			m_idle_session.pop_front();
		}
		Timestamp t(timestamp);
		m_active_session[session_id] = t;
	}
	else
	{
		Timestamp t(timestamp);
		it_map->second = t;
	}
	return session_id;

}
*/
bool CRPCServer::addObserver(const AbstractObserver& o)
{
	m_center.addObserver(o);
	return true;
}

bool CRPCServer::removeObserver(const AbstractObserver& o)
{
	m_center.removeObserver(o);
	tracef("%s, %d: remove observer\n", __FILE__, __LINE__);
	return true;
}

void CRPCServer::run()
{
	for(;;)
	{
		Notification::Ptr pNf = m_queue.waitDequeueNotification();
		if(pNf)
		{
			Mutex::ScopedLock lock(m_mutex);
			RequestNotification* pR = pNf.cast<RequestNotification>();
			CRPCClient* client;
			JSON::Object::Ptr pParam = pR->getParam();
			DynamicStruct ds = *pParam;
			client = new CRPCClient();
			client->reset();
			client->setRequest(pR);
			m_task_manager->start(client);
		}
		else
		{
			errorf("%s, %d: Not supposed to be here.\n", __FILE__, __LINE__);
		}
	}
}

void CRPCServer::handleFinish(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		Mutex::ScopedLock lock(m_mutex);
		CRPCClient* client = (CRPCClient*)p->task();
		RequestNotification::Ptr pResult = client->getResult();
		JSON::Object::Ptr result = pResult->getParam();
		int id = pResult->getID();
		DynamicStruct ds = *result;
		std::string param = ds.toString();
		tracef("%s, %d: Handle finish, result:%s.\n", __FILE__, __LINE__, param.c_str());
		try
		{
			m_center.postNotification(new RequestNotification(id, result));
			tracepoint();
		}
		catch(Exception& e)
		{
			tracef("%s, %d: %s\n", __FILE__, __LINE__, e.message().c_str());
		}
		client->release();
	}
	else
	{
		errorf("%s, %d: Not supposed to be here.\n", __FILE__, __LINE__);
	}
}

