#include "Device/RPC/RPCServer.h"
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
		warnf("%s, %d: RPCServer is already started.", __FILE__, __LINE__);
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
		warnf("%s, %d: RPCServer is not started.", __FILE__, __LINE__);
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

bool CRPCServer::addObserver(const AbstractObserver& o)
{
	m_center.addObserver(o);
	return true;
}

bool CRPCServer::removeObserver(const AbstractObserver& o)
{
	m_center.removeObserver(o);
	return true;
}

void CRPCServer::run()
{
	for(;;)
	{
		Notification::Ptr pNf = m_queue.waitDequeueNotification();
		if(pNf)
		{
			RequestNotification* pR = pNf.cast<RequestNotification>();
			CRPCClient* client;
			std::string request = pR->getRequest();
			UInt64 id = pR->getID();
			client = new CRPCClient();
			client->reset();
			client->setRequest(request);
			client->setID(id);
			m_task_manager->start(client);
		}
		else
		{
			errorf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
		}
	}
}

void CRPCServer::handleFinish(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		CRPCClient* client = (CRPCClient*)p->task();
		JSON::Object::Ptr response = client->getResponse();
		UInt64 id = client->getID();
		if(!response.isNull())
		{
			DynamicStruct ds = *response;
			std::string param = ds.toString();
			tracef("%s, %d: Handle finish, result:%s.", __FILE__, __LINE__, param.c_str());
			m_center.postNotification(new RequestNotification(id, "", response));
		}
		client->release();
	}
	else
	{
		errorf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
	}
}

