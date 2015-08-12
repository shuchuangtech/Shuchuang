#include "TransmitServer/RegServer.h"
#include "TransmitServer/RegMsgHandler.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/Net/SocketImpl.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/Context.h"
#include "Poco/Timespan.h"
#include "Poco/Observer.h"
#include "TransmitServer/DeviceManager.h"
#include "Common/ConfigManager.h"
CRegServer::CRegServer()
{
	m_started = false;
	m_ssl_port = 0;
	m_reg_port = 0;
	m_ssl_sock = NULL;
	m_reg_sock = NULL;
	m_ssl_thread_pool = NULL;
	m_reg_thread_pool = NULL;
	m_ssl_task_manager = NULL;
	m_reg_task_manager = NULL;
	m_pSsl_map.clear();
	m_pReg_map.clear();
	m_request_map.clear();
	m_ssl_sock_list.clear();
	m_reg_sock_list.clear();
}

CRegServer::~CRegServer()
{
	if(m_ssl_thread_pool != NULL)
		delete m_ssl_thread_pool;
	if(m_ssl_task_manager != NULL)
		delete m_ssl_task_manager;
	if(m_reg_thread_pool != NULL)
		delete m_reg_thread_pool;
	if(m_reg_task_manager != NULL)
		delete m_reg_task_manager;
}

void CRegServer::start()
{
	if(m_started)
	{
		warnf("%s, %d: Register server has already started.", __FILE__, __LINE__);
		return;
	}
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pConfig;
	config->getConfig("RegServer", pConfig);
	if(pConfig.isNull())
	{
		pConfig = new JSON::Object;
		pConfig->set("ssl_port", 12222);
		pConfig->set("reg_port", 13333);
		config->setConfig("RegServer", pConfig);
		m_ssl_port = 12222;
		m_reg_port = 13333;
	}
	else
	{
		if(pConfig->has("ssl_port") && pConfig->has("reg_port"))
		{
			m_ssl_port = pConfig->getValue<UInt16>("ssl_port");
			m_reg_port = pConfig->getValue<UInt16>("reg_port");
		}
		else
		{
			m_ssl_port = 12222;
			m_reg_port = 13333;
			pConfig = NULL;
			pConfig = new JSON::Object;
			pConfig->set("ssl_port", 12222);
			pConfig->set("reg_port", 13333);
			config->setConfig("RegServer", pConfig);
		}
	}
	std::string ssl_tp_name = "SslThreadPool";
	std::string reg_tp_name = "RegThreadPool";
	m_ssl_thread_pool = new ThreadPool(ssl_tp_name);
	m_ssl_task_manager = new TaskManager(*m_ssl_thread_pool);
	m_reg_thread_pool = new ThreadPool(reg_tp_name);
	m_reg_task_manager = new TaskManager(*m_reg_thread_pool);
	Observer<CRegServer, TaskFinishedNotification> ssl_observer(*this, &CRegServer::handleSslFinish);
	m_ssl_task_manager->addObserver(ssl_observer);
	Observer<CRegServer, TaskFinishedNotification> reg_observer(*this, &CRegServer::handleRegFinish);
	m_reg_task_manager->addObserver(reg_observer);
	Observer<CRegServer, OfflineNotification> device_observer(*this, &CRegServer::handleOffline);
	CDeviceManager* device_manager = CDeviceManager::instance();
	device_manager->addObserver(device_observer);
	listenSsl();
	listenReg();
	m_started = true;
	TimerCallback<CRegServer> sslCallback(*this, &CRegServer::sslAccept);
	m_ssl_accept.start(sslCallback);
	
	TimerCallback<CRegServer> sslHanCall(*this, &CRegServer::sslHandler);
	m_ssl_handler.start(sslHanCall);

	TimerCallback<CRegServer> regCallback(*this, &CRegServer::regAccept);
	m_reg_accept.start(regCallback);
	TimerCallback<CRegServer> regHanCall(*this, &CRegServer::regHandler);
	m_reg_handler.start(regHanCall);
	infof("%s, %d: Register server start successfully.", __FILE__, __LINE__);
	infof("%s, %d: Register server ssl port:%d, reg port:%d", __FILE__, __LINE__, m_ssl_port, m_reg_port);
}

void CRegServer::stop()
{
	if(!m_started)
		return;
	m_started = false;
	m_ssl_accept.stop();
	m_reg_accept.stop();
	m_ssl_handler.stop();
	m_reg_handler.stop();
	m_ssl_sock->close();
	delete m_ssl_sock;
	m_reg_sock->close();
	delete m_reg_sock;
	m_ssl_thread_pool->stopAll();
	m_reg_thread_pool->stopAll();
	infof("%s, %d: Register server stop successfully.", __FILE__, __LINE__);
}

bool CRegServer::listenSsl()
{
	Context::Ptr pContext = NULL;
	try
	{
		pContext = new Context(Context::TLSV1_SERVER_USE,
							"./privkey.pem",
							"./cert.pem",
							"",
							Context::VERIFY_NONE);
	}
	catch(Exception& e)
	{
		return false;
	}
	try
	{
		m_ssl_sock = new SecureServerSocket(m_ssl_port, 64, pContext);
	}
	catch(Exception& e)
	{
		if(m_ssl_sock != NULL)
			delete m_ssl_sock;
		return false;
	}
	return true;
}

bool CRegServer::listenReg()
{
	try
	{
		m_reg_sock = new ServerSocket(m_reg_port);
	}
	catch(Exception& e)
	{
		if(m_reg_sock != NULL)
			delete m_reg_sock;
		return false;
	}
	return true;
}

void CRegServer::handleOffline(OfflineNotification* pNf)
{
	OfflineNotification::Ptr p(pNf);
	if(p)
	{
		UInt64 impl = p->getID();
		std::map<UInt64, SocketTime*>::iterator it = m_pReg_map.find(impl);
		if(it != m_pReg_map.end())
		{
			debugf("%s, %d: Connection %s shutdown by server.", __FILE__, __LINE__, it->second->socket.peerAddress().toString().c_str());
			removeSocket(1, it);
		}
	}
}

void CRegServer::sslAccept(Timer& timer)
{
	while(m_started)
	{
		if(m_ssl_sock == NULL)
		{
			Thread::sleep(500);
			continue;
		}
		SocketAddress clientAddress;
		StreamSocket ss;
		while(true)
		{
			if(m_ssl_sock->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
			{
				ss = m_ssl_sock->acceptConnection(clientAddress);
				break;
			}
			else
			{
				if(!m_started)
				{
					break;
				}
			}
		}
		if(!m_started)
		{
			break;
		}
		tracef("%s, %d: Ssl accept connection from %s.", __FILE__, __LINE__, clientAddress.toString().c_str());
		Timestamp t;
		SocketTime* pSsl = new SocketTime(ss, t);
		Mutex::ScopedLock lock(m_ssl_queue_mutex);
		m_pSsl_map.insert(std::make_pair<UInt64, SocketTime*>((UInt64)ss.impl() ,pSsl));
		m_ssl_sock_list.push_back(ss);
	}
}

void CRegServer::sslHandler(Timer& timer)
{
	Timestamp lastCheckTime;
	Timestamp::TimeDiff checkPeriod = 5 * 1000 * 1000;
	Socket::SocketList readList;
	Socket::SocketList writeList;
	Socket::SocketList errorList;
	while(m_started)
	{
		m_ssl_queue_mutex.lock();
		if(m_ssl_sock_list.empty())
		{
			//tracef("list size:%d\n", m_ssl_sock_list.size());
			m_ssl_queue_mutex.unlock();
			Thread::sleep(500);
			continue;
		}
		writeList.clear();
		readList.clear();
		std::copy(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), std::back_inserter(readList));
		errorList.clear();
		std::copy(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), std::back_inserter(errorList));
		m_ssl_queue_mutex.unlock();
		int ret = Socket::select(readList, writeList, errorList, Timespan(5, 0));
		if( ret > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				//handle
				char buf[512] = {0, };
				StreamSocket ss((*it));
				UInt64 impl = (UInt64)it->impl();
				std::map<UInt64, SocketTime*>::iterator it_ssl = m_pSsl_map.find(impl);
				if(ss.receiveBytes(buf, sizeof(buf)) > 0)
				{
					CRegMsgHandler* msgHandler = new CRegMsgHandler(0);
					msgHandler->setParam(impl, buf, sizeof(buf), ss);
					m_ssl_task_manager->start(msgHandler);
					Timestamp t;
					if(it_ssl != m_pSsl_map.end())
					{
						it_ssl->second->time = t;
					}
				}
				else
				{
					tracef("%s, %d: Client %s disconnect.",__FILE__, __LINE__,  (it->peerAddress()).toString().c_str());
					if(it_ssl != m_pSsl_map.end())
						removeSocket(0, it_ssl);
				}	
			}// for readList
		}//select ret
		Timestamp now;
		if(now - lastCheckTime < checkPeriod)
			continue;
		for(std::map<UInt64, SocketTime*>::iterator it_ssl = m_pSsl_map.begin(); it_ssl != m_pSsl_map.end(); )
		{
			std::map<UInt64, SocketTime*>::iterator it_temp = it_ssl++;
			if(it_temp->second->time - now > 12 * checkPeriod)
			{
				removeSocket(0, it_temp);
			}
		}
	}
}

void CRegServer::handleSslFinish(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		CRegMsgHandler* regMsgHandler = (CRegMsgHandler*)p->task();
		UInt64 id = regMsgHandler->getId();
		JSON::Object::Ptr pObj = regMsgHandler->getResult();
		DynamicStruct ds = *pObj;
		//delete regMsgHandler;
		std::map<UInt64, SocketTime*>::iterator it = m_pSsl_map.find(id);
		if(it != m_pSsl_map.end())
		{
			SocketTime* pSsl = it->second;
			StreamSocket ss(pSsl->socket);
			ss.sendBytes(ds.toString().c_str(), ds.toString().length());
			removeSocket(0, it);
			tracef("%s, %d: Ssl result sent[%s].", __FILE__, __LINE__, ds.toString().c_str());
		}
	}
}

void CRegServer::handleRegFinish(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		CRegMsgHandler* regMsgHandler = (CRegMsgHandler*)p->task();
		UInt64 reg_id = regMsgHandler->getId();
		JSON::Object::Ptr pObj = regMsgHandler->getResult();
		if(pObj.isNull())
		{
				errorf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
			return;
		}
		DynamicStruct ds = *pObj;
		if(ds.contains(KEY_TYPE_STR) && ds[KEY_TYPE_STR].toString() == TYPE_RESPONSE_STR)
		{
			UInt64 request_id;
			request_id = ds["requestid"];
			m_request_queue_mutex.lock();
			std::map<UInt64, RequestInfo*>::iterator it_req = m_request_map.find(request_id);
			if(it_req != m_request_map.end())
			{
				RequestInfo* request = it_req->second;
				m_request_map.erase(it_req);
				request->response = pObj;
				request->sem.set();
			}
			else
			{
				errorf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
			}
			m_request_queue_mutex.unlock();
			return;
		}
		std::map<UInt64, SocketTime*>::iterator it = m_pReg_map.find(reg_id);
		if(it != m_pReg_map.end())
		{
			SocketTime* pReg = it->second;
			StreamSocket ss(pReg->socket);
			ss.sendBytes(ds.toString().c_str(), ds.toString().length());
			tracef("%s, %d: Reg result sent[%s].", __FILE__, __LINE__, ds.toString().c_str());
		}
	}
}

bool CRegServer::removeSocket(int choice, std::map<UInt64, SocketTime*>::iterator it)
{
	if(choice == 0)
	{
		m_ssl_queue_mutex.lock();
		Socket::SocketList::iterator it_del = find(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), it->second->socket);
		if(it_del != m_ssl_sock_list.end())
		{
			m_ssl_sock_list.erase(it_del);
		}
		it->second->socket.close();
		delete it->second;
		m_pSsl_map.erase(it);
		m_ssl_queue_mutex.unlock();
		return true;
	}
	else if(choice == 1)
	{
		m_reg_queue_mutex.lock();
		Socket::SocketList::iterator it_del = find(m_reg_sock_list.begin(), m_reg_sock_list.end(), it->second->socket);
		if(it_del != m_reg_sock_list.end())
		{
			m_reg_sock_list.erase(it_del);
		}
		it->second->socket.close();
		delete it->second;
		m_pReg_map.erase(it);
		m_reg_queue_mutex.unlock();
		return true;
	}
	return false;
}

void CRegServer::regHandler(Timer& timer)
{
	Timestamp lastCheckTime;
	Timestamp::TimeDiff checkPeriod = 5 * 1000 * 1000;
	Socket::SocketList readList;
	Socket::SocketList writeList;
	Socket::SocketList errorList;
	while(m_started)
	{
		m_reg_queue_mutex.lock();
		if(m_reg_sock_list.empty())
		{
			m_reg_queue_mutex.unlock();
			Thread::sleep(500);
			continue;
		}
		writeList.clear();
		readList.clear();
		std::copy(m_reg_sock_list.begin(), m_reg_sock_list.end(), std::back_inserter(readList));
		errorList.clear();
		std::copy(m_reg_sock_list.begin(), m_reg_sock_list.end(), std::back_inserter(errorList));
		m_reg_queue_mutex.unlock();
		int ret = Socket::select(readList, writeList, errorList, Timespan(5, 0));
		if(ret > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				char buf[512] = {0, };
				StreamSocket ss((*it));
				UInt64 impl = (UInt64)it->impl();
				std::map<UInt64, SocketTime*>::iterator it_reg = m_pReg_map.find(impl);
				if(ss.receiveBytes(buf, sizeof(buf)) > 0)
				{
					CRegMsgHandler* msgHandler = new CRegMsgHandler(1);
					msgHandler->setParam(impl, buf, sizeof(buf), ss);
					m_reg_task_manager->start(msgHandler);
					Timestamp t;
					if(it_reg != m_pReg_map.end())
					{
						it_reg->second->time = t;
					}
				}
				else
				{
					infof("%s, %d: Client %s disconnect.", __FILE__, __LINE__, (it->peerAddress()).toString().c_str());
					CDeviceManager* dev_manager = CDeviceManager::instance();
					dev_manager->deviceOffline(impl);
					if(it_reg != m_pReg_map.end())
					{
						removeSocket(1, it_reg);
					}
				}
			}// for readList
		}//select ret
		Timestamp now;
		if(now - lastCheckTime < checkPeriod)
			continue;
		Timestamp t;
		lastCheckTime = t;
		for(std::map<UInt64, SocketTime*>::iterator it_reg = m_pReg_map.begin(); it_reg != m_pReg_map.end(); )
		{
			std::map<UInt64, SocketTime*>::iterator it_temp = it_reg++;
			if(it_temp->second->time - now > 12 * checkPeriod)
			{
				removeSocket(1, it_temp);
			}
		}
		m_request_queue_mutex.lock();
		for(std::map<UInt64, RequestInfo*>::iterator it = m_request_map.begin(); it != m_request_map.end(); )
		{
			infof("%s, %d: check http request timeout", __FILE__, __LINE__);
			std::map<UInt64, RequestInfo*>::iterator itemp = it++;
			if(now - itemp->first > itemp->second->timeout)
			{
				infof("%s, %d: Request %lu timeout.", __FILE__, __LINE__, itemp->first);
				itemp->second->sem.set();
				m_request_map.erase(itemp);
			}
		}
		m_request_queue_mutex.unlock();
	}
}

void CRegServer::regAccept(Timer& timer)
{
	while(m_started)
	{
		if(m_reg_sock == NULL)
		{
			Thread::sleep(500);
			continue;
		}
		SocketAddress clientAddress;
		StreamSocket ss;
		while(true)
		{
			if(m_reg_sock->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
			{
				ss = m_reg_sock->acceptConnection(clientAddress);
				break;
			}
			else
			{
				if(!m_started)
				{
					break;
				}
			}
		}
		if(!m_started)
		{
			break;
		}
		infof("%s, %d: Reg accept connection from %s.", __FILE__, __LINE__, clientAddress.toString().c_str());
		Timestamp t;
		SocketTime* pReg = new SocketTime(ss, t);
		Mutex::ScopedLock lock(m_reg_queue_mutex);
		tracef("%s, %d: insert [%lu] to reg map", __FILE__, __LINE__, (UInt64)ss.impl());
		m_pReg_map.insert(std::make_pair<UInt64, SocketTime*>((UInt64)ss.impl() ,pReg));
		m_reg_sock_list.push_back(ss);
	}
}

bool CRegServer::sendRequest(RequestInfo* request)
{
	DynamicStruct req = *(request->request);
	UInt64 src = request->src_id;
	std::string dst = request->uuid;
	DynamicStruct ds = *(request->request);
	CDeviceManager* dev_manager = CDeviceManager::instance();
	DeviceInfo* dev = dev_manager->getDevice(request->uuid);	
	if(dev == NULL)
	{
		JSON::Object::Ptr obj = new JSON::Object(*(request->request));
		obj->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		obj->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		obj->set(KEY_DETAIL_STR, "Device not found");
		request->response = obj;
		return false;
	}
	int id = dev->id;
	std::map<UInt64, SocketTime*>::iterator it = m_pReg_map.find(id);
	if(it == m_pReg_map.end())
	{
		JSON::Object::Ptr obj = new JSON::Object(*(request->request));
		obj->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		obj->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		obj->set(KEY_DETAIL_STR, "Device not found");
		request->response = obj;
		return false;
	}
	Timestamp t;
	UInt64 request_id = t.epochMicroseconds();
	m_request_queue_mutex.lock();
	m_request_map.insert(std::make_pair<UInt64, RequestInfo*>(request_id, request));
	m_request_queue_mutex.unlock();
	ds[KEY_REQUEST_ID_STR] = request_id;
	tracef("%s, %d: Request sent to device.", __FILE__, __LINE__);
	it->second->socket.sendBytes(ds.toString().c_str(), ds.toString().length());
	request->sem.wait();
	return true;
}

bool CRegServer::setServerPort(UInt16 ssl_port, UInt16 reg_port)
{
	if(ssl_port < 1024 || ssl_port > 65535 || reg_port < 1024 || reg_port > 65535)
		return false;
	m_ssl_port = ssl_port;
	m_reg_port = reg_port;
	return true;
}

