#include "TransmitServer/RegServer/RegServer.h"
#include "TransmitServer/RegServer/RegMsgHandler.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/Net/SocketImpl.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/Context.h"
#include "Poco/Timespan.h"
#include "Poco/Observer.h"
#include "TransmitServer/DeviceManager.h"
#include "Common/ConfigManager.h"
using namespace Poco;
using namespace Poco::Net;
CRegServer::CRegServer()
{
	m_started = false;
	m_ssl_port = 0;
	m_reg_port = 0;
	m_ssl_sock = NULL;
	m_reg_sock = NULL;
	m_task_manager = NULL;
	m_task_manager = NULL;
	m_pSsl_map.clear();
	m_pReg_map.clear();
	m_request_map.clear();
	m_ssl_sock_list.clear();
	m_reg_sock_list.clear();
}

CRegServer::~CRegServer()
{
	if(m_thread_pool != NULL)
		delete m_thread_pool;
	if(m_task_manager != NULL)
		delete m_task_manager;
}

bool CRegServer::start()
{
	if(m_started)
	{
		warnf("%s, %d: Register server has already started.", __FILE__, __LINE__);
		return false;
	}
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pConfig;
	config->getConfig("RegServer", pConfig);
	if(pConfig.isNull() || !pConfig->has("ssl_port") || !pConfig->has("reg_port"))
	{
		warnf("%s, %d: RegServer config not exists, or ssl_port reg_port not exists.", __FILE__, __LINE__);
		return false;
	}
	m_ssl_port = pConfig->getValue<UInt16>("ssl_port");
	m_reg_port = pConfig->getValue<UInt16>("reg_port");
	createInnerSocket();
	listenSsl();
	listenReg();
	std::string tp_name = "RegThreadPool";
	m_thread_pool = new ThreadPool(tp_name);
	m_task_manager = new TaskManager(*m_thread_pool);
	Observer<CRegServer, TaskFinishedNotification> task_observer(*this, &CRegServer::handleTaskFinish);
	m_task_manager->addObserver(task_observer);
	Observer<CRegServer, OfflineNotification> device_observer(*this, &CRegServer::handleOffline);
	CDeviceManager* device_manager = CDeviceManager::instance();
	device_manager->addObserver(device_observer);
	m_started = true;
	TimerCallback<CRegServer> sslCallback(*this, &CRegServer::sslAccept);
	m_ssl_accept.start(sslCallback);

	TimerCallback<CRegServer> httpCallback(*this, &CRegServer::httpRequestTimer);
	m_http_request_timer.start(httpCallback);

	TimerCallback<CRegServer> sslHanCall(*this, &CRegServer::sslHandler);
	m_ssl_handler.start(sslHanCall);

	TimerCallback<CRegServer> regCallback(*this, &CRegServer::regAccept);
	m_reg_accept.start(regCallback);

	TimerCallback<CRegServer> regHanCall(*this, &CRegServer::regHandler);
	m_reg_handler.start(regHanCall);
	infof("%s, %d: Register server start successfully.", __FILE__, __LINE__);
	infof("%s, %d: Register server ssl port:%d, reg port:%d", __FILE__, __LINE__, m_ssl_port, m_reg_port);
	return true;
}

bool CRegServer::stop()
{
	if(!m_started)
		return false;
	m_started = false;
	m_ssl_accept.stop();
	m_reg_accept.stop();
	m_ssl_handler.stop();
	m_reg_handler.stop();
	m_http_request_timer.stop();
	m_ssl_sock->close();
	delete m_ssl_sock;
	m_reg_sock->close();
	delete m_reg_sock;
	m_thread_pool->stopAll();
	infof("%s, %d: Register server stop successfully.", __FILE__, __LINE__);
	return true;
}

bool CRegServer::createInnerSocket()
{
	Timer timer;
	TimerCallback<CRegServer> innerCallback(*this, &CRegServer::handleInnerSocket);
	timer.start(innerCallback);
	Thread::sleep(500);
	try
	{
		m_inner_ssl_read_socket.connect(SocketAddress("127.0.0.1", 9701), Timespan(5, 0));
		m_inner_reg_read_socket.connect(SocketAddress("127.0.0.1", 9701), Timespan(5, 0));
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Create inner read socket error[%s].", __FILE__, __LINE__, e.message().c_str());
		timer.stop();
		return false;
	}
	timer.stop();
	m_ssl_sock_list.push_back(m_inner_ssl_read_socket);
	m_reg_sock_list.push_back(m_inner_reg_read_socket);
	infof("%s, %d: Create inner ssl read socket[%s] successfully.", __FILE__, __LINE__, m_inner_ssl_read_socket.address().toString().c_str());
	infof("%s, %d: Create inner reg read socket[%s] successfully.", __FILE__, __LINE__, m_inner_reg_read_socket.address().toString().c_str());
	return true;
}

bool CRegServer::writeInnerSocket(int choice)
{
	if(choice == 0)
	{
		m_inner_ssl_socket_mutex.lock();
		m_inner_ssl_write_socket.sendBytes("A", 1);
		m_inner_ssl_socket_mutex.unlock();
	}
	else if(choice == 1)
	{
		m_inner_reg_socket_mutex.lock();
		m_inner_reg_write_socket.sendBytes("A", 1);
		m_inner_reg_socket_mutex.unlock();
	}
	return true;
}

bool CRegServer::readInnerSocket(int choice)
{
	char buf[2] = {0, };
	if(choice == 0)
	{
		m_inner_ssl_read_socket.receiveBytes(buf, 1);
	}
	else if(choice == 1)
	{
		m_inner_reg_read_socket.receiveBytes(buf, 1);
	}
	return true;
}

void CRegServer::handleInnerSocket(Timer& timer)
{
	//inner socket port 9701
	ServerSocket* svr = NULL;
	try
	{
		svr = new ServerSocket(9701);
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Create inner read socket error[%s].", __FILE__, __LINE__, e.message().c_str());
		if(svr != NULL)
			delete svr;
		return;
	}
	while(1)
	{
		if(svr->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			SocketAddress sa;
			m_inner_ssl_write_socket = svr->acceptConnection(sa);
			infof("%s, %d: Create inner ssl write socket[%s] successfully.", __FILE__, __LINE__, m_inner_ssl_write_socket.address().toString().c_str());
			break;
		}
	}
	while(1)
	{
		if(svr->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
		{
			SocketAddress sa;
			m_inner_reg_write_socket = svr->acceptConnection(sa);
			infof("%s, %d: Create inner reg write socket[%s] successfully.", __FILE__, __LINE__, m_inner_reg_write_socket.address().toString().c_str());
			break;
		}
	}
	delete svr;
	return;
}

bool CRegServer::listenSsl()
{
	Context::Ptr pContext = NULL;
	try
	{
		pContext = new Context(Context::TLSV1_SERVER_USE,
							"./my.key",
							"./my.crt",
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
		errorf("%s, %d: Listen port %u failed.", __FILE__, __LINE__, m_ssl_port);
		return false;
	}
	return true;
}

bool CRegServer::listenReg()
{
	Context::Ptr pContext = NULL;
	try
	{
		pContext = new Context(Context::TLSV1_SERVER_USE,
								"./my.key",
								"./my.crt",
								"",
								Context::VERIFY_NONE);
	}
	catch(Exception& e)
	{
		return false;
	}
	try
	{
		m_reg_sock = new SecureServerSocket(m_reg_port, 64, pContext);
	}
	catch(Exception& e)
	{
		if(m_reg_sock != NULL)
			delete m_reg_sock;
		errorf("%s, %d: Listen port %u failed.", __FILE__, __LINE__, m_reg_port);
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
		m_reg_queue_mutex.lock();
		std::map<UInt64, SocketTime*>::iterator it = m_pReg_map.find(impl);
		if(it != m_pReg_map.end())
		{
			debugf("%s, %d: Connection %s shutdown by server.", __FILE__, __LINE__, it->second->saddr.toString().c_str());
			removeSocket(1, it);
		}
		m_reg_queue_mutex.unlock();
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
			break;
		infof("%s, %d: Ssl accept connection from %s to port %u.", __FILE__, __LINE__, clientAddress.toString().c_str(), ss.address().port());
		Timestamp t;
		SocketTime* pSsl = new SocketTime(ss, t);
		Mutex::ScopedLock lock(m_ssl_queue_mutex);
		m_pSsl_map.insert(std::make_pair<UInt64, SocketTime*>((UInt64)ss.impl() ,pSsl));
		m_ssl_sock_list.push_back(ss);
		writeInnerSocket(0);
	}
}

void CRegServer::sslHandler(Timer& timer)
{
	Socket::SocketList readList;
	Socket::SocketList writeList;
	Socket::SocketList errorList;
	while(m_started)
	{
		m_ssl_queue_mutex.lock();
		writeList.clear();
		readList.clear();
		std::copy(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), std::back_inserter(readList));
		errorList.clear();
		std::copy(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), std::back_inserter(errorList));
		m_ssl_queue_mutex.unlock();
		int ret = Socket::select(readList, writeList, errorList, Timespan(60, 0));
		if( ret > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				if(*it == m_inner_ssl_read_socket)
				{
					readInnerSocket(0);
					continue;
				}
				m_ssl_queue_mutex.lock();
				Socket::SocketList::iterator it_ssl = find(m_ssl_sock_list.begin(), m_ssl_sock_list.end(), *it);
				if(it_ssl != m_ssl_sock_list.end())
				{
					m_ssl_sock_list.erase(it_ssl);
				}
				std::map<UInt64, SocketTime*>::iterator it_pSsl = m_pSsl_map.find((UInt64)(*it).impl());
				if(it_pSsl != m_pSsl_map.end())
				{
					CRegMsgHandler* msgHandler = new CRegMsgHandler(0);
					msgHandler->setSocket(it_pSsl->second);
					m_pSsl_map.erase(it_pSsl);
					m_task_manager->start(msgHandler);
				}
				m_ssl_queue_mutex.unlock();
			}// for readList
		}//select ret
	}
}

void CRegServer::handleTaskFinish(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		CRegMsgHandler* regMsgHandler = (CRegMsgHandler*)p->task();
		int type = regMsgHandler->getType();
		if(type == 0)
			//ssl
		{
			SocketTime* st = regMsgHandler->getSocket();
			try
			{
				infof("%s, %d: socket[%s] closed.", __FILE__, __LINE__, st->saddr.toString().c_str());
				st->socket.close();
			}
			catch(Exception& e)
			{
				warnf("%s, %d: socket error[%s].", __FILE__, __LINE__, e.message().c_str());
			}
			delete st;
		}
		else if(type == 1)
			//reg
		{
			UInt64 req_id = regMsgHandler->getRequestID();
			SocketTime* st = regMsgHandler->getSocket();
			if(req_id == 0)
				//reg request
			{
				if(!regMsgHandler->socketReceive())
				{
					//disconnect
					infof("%s, %d: socket[%s] closed.", __FILE__, __LINE__, st->saddr.toString().c_str());
					st->socket.close();
					delete st;
					st = NULL;
				}
				else
				{
					m_reg_queue_mutex.lock();
					m_reg_sock_list.push_back(st->socket);
					UInt64 impl = (UInt64)st->socket.impl();
					m_pReg_map.insert(std::make_pair<UInt64, SocketTime*>(impl, st));
					m_reg_queue_mutex.unlock();
					writeInnerSocket(1);
				}
			}
			else
				//http response
			{
				m_reg_queue_mutex.lock();
				m_reg_sock_list.push_back(st->socket);
				UInt64 impl = (UInt64)st->socket.impl();
				m_pReg_map.insert(std::make_pair<UInt64, SocketTime*>(impl, st));
				m_reg_queue_mutex.unlock();
				if(req_id == 1)
				{
					warnf("%s, %d: Request id error.", __FILE__, __LINE__);
				}
				else
				{
					m_request_queue_mutex.lock();
					std::map<UInt64, RequestInfo*>::iterator it_req = m_request_map.find(req_id);
					if(it_req != m_request_map.end())
					{
						RequestInfo* request = it_req->second;
						m_request_map.erase(it_req);
						request->response = regMsgHandler->getHTTPResponse(); 
						request->sem.set();
					}
					m_request_queue_mutex.unlock();
				}
				writeInnerSocket(1);
			}
		}//type == 1
	}//if p
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
	Socket::SocketList readList;
	Socket::SocketList writeList;
	Socket::SocketList errorList;
	while(m_started)
	{
		m_reg_queue_mutex.lock();
		writeList.clear();
		readList.clear();
		std::copy(m_reg_sock_list.begin(), m_reg_sock_list.end(), std::back_inserter(readList));
		errorList.clear();
		std::copy(m_reg_sock_list.begin(), m_reg_sock_list.end(), std::back_inserter(errorList));
		m_reg_queue_mutex.unlock();
		//may result in http response hung up
		int ret = Socket::select(readList, writeList, errorList, Timespan(60, 0));
		if(ret > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				if(*it == m_inner_reg_read_socket)
				{
					readInnerSocket(1);
					continue;
				}
				m_reg_queue_mutex.lock();
				Socket::SocketList::iterator it_reg = find(m_reg_sock_list.begin(), m_reg_sock_list.end(), *it);
				if(it_reg != m_reg_sock_list.end())
				{
					m_reg_sock_list.erase(it_reg);
				}
				UInt64 impl = (UInt64)it->impl();
				std::map<UInt64, SocketTime*>::iterator it_pReg = m_pReg_map.find(impl);
				if(it_pReg != m_pReg_map.end())
				{
					CRegMsgHandler* msgHandler = new CRegMsgHandler(1);
					msgHandler->setSocket(it_pReg->second);
					m_pReg_map.erase(it_pReg);
					m_task_manager->start(msgHandler);
				}
				m_reg_queue_mutex.unlock();
			}// for readList
		}//select ret
	}
}

void CRegServer::httpRequestTimer(Timer& timer)
{
	Timestamp lastCheckTime;
	Timestamp::TimeDiff checkPeriod = 5 * 1000 * 1000;
	while(m_started)
	{
		Timestamp now;
		if(now - lastCheckTime < checkPeriod)
		{
			Thread::sleep(5000);
			continue;
		}
		lastCheckTime = now;
		m_request_queue_mutex.lock();
		for(std::map<UInt64, RequestInfo*>::iterator it = m_request_map.begin(); it != m_request_map.end(); )
		{
			infof("%s, %d: check http request timeout", __FILE__, __LINE__);
			std::map<UInt64, RequestInfo*>::iterator itemp = it++;
			if(now - itemp->first > itemp->second->timeout)
			{
				infof("%s, %d: Request %llu timeout.", __FILE__, __LINE__, itemp->second->src_id);
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
		m_pReg_map.insert(std::make_pair<UInt64, SocketTime*>((UInt64)ss.impl() ,pReg));
		m_reg_sock_list.push_back(ss);
		writeInnerSocket(1);
	}
}

bool CRegServer::sendRequest(RequestInfo* request)
{
	DynamicStruct ds = *(request->request);
	CDeviceManager* dev_manager = CDeviceManager::instance();
	DeviceInfo* dev = dev_manager->getDevice(request->uuid);
	if(dev == NULL)
	{
		JSON::Object::Ptr obj = new JSON::Object(*(request->request));
		obj->remove(KEY_TYPE_STR);
		obj->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		obj->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		obj->set(KEY_DETAIL_STR, "404");
		request->response = obj;
		return false;
	}
	UInt64 id = dev->id;
	std::map<UInt64, SocketTime*>::iterator it = m_pReg_map.find(id);
	if(it == m_pReg_map.end())
	{
		JSON::Object::Ptr obj = new JSON::Object(*(request->request));
		obj->remove(KEY_TYPE_STR);
		obj->set(KEY_TYPE_STR, TYPE_RESPONSE_STR);
		obj->set(KEY_RESULT_STR, RESULT_FAIL_STR);
		obj->set(KEY_DETAIL_STR, "404");
		request->response = obj;
		return false;
	}
	Timestamp t;
	UInt64 request_id = t.epochMicroseconds();
	m_request_queue_mutex.lock();
	m_request_map.insert(std::make_pair<UInt64, RequestInfo*>(request_id, request));
	m_request_queue_mutex.unlock();
	ds[KEY_REQUEST_ID_STR] = request_id;
	it->second->socket.sendBytes(ds.toString().c_str(), ds.toString().length());
	tracef("%s, %d: Request %s sent to device.", __FILE__, __LINE__, ds.toString().c_str());
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

