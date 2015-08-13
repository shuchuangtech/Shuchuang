#ifndef __SERVER_REGSERVER_H__
#define __SERVER_REGSERVER_H__
#include <vector>
#include <map>
#include "Poco/SingletonHolder.h"
#include "Poco/Types.h"
#include "Poco/Timestamp.h"
#include "Poco/Timer.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Mutex.h"
#include "Poco/TaskManager.h"
#include "Poco/ThreadPool.h"
#include "Poco/TaskNotification.h"
#include "TransmitServer/RequestInfo.h"
#include "TransmitServer/SocketTime.h"
#include "TransmitServer/OfflineNotification.h"
using namespace Poco;
using namespace Poco::Net;
class CRegServer
{
public:
	static CRegServer* instance()
	{
		static SingletonHolder<CRegServer> sh;
		return sh.get();
	};
	CRegServer();
	~CRegServer();
	void start();
	void stop();
	bool setServerPort(UInt16 ssl_port, UInt16 reg_port);
	void sslAccept(Timer& timer);
	void sslHandler(Timer& timer);
	void regAccept(Timer& timer);
	void regHandler(Timer& timer);
	void handleSslFinish(TaskFinishedNotification* pNf);
	void handleRegFinish(TaskFinishedNotification* pNf);
	bool sendRequest(RequestInfo* request);
	void handleOffline(OfflineNotification* pNf);
private:
	bool removeSocket(int choice, std::map<UInt64, SocketTime*>::iterator it);
	//0 ssl, 1 reg
	bool					listenSsl();
	bool					listenReg();
	bool					m_started;
	UInt16					m_ssl_port;
	UInt16					m_reg_port;
	Timer					m_ssl_accept;
	Timer					m_reg_accept;
	Timer					m_ssl_handler;
	Timer					m_reg_handler;
	SecureServerSocket*		m_ssl_sock;
	ServerSocket*			m_reg_sock;
	std::map<UInt64, SocketTime*>	m_pSsl_map;
	std::map<UInt64, SocketTime*>	m_pReg_map;
	std::map<UInt64, RequestInfo*>		m_request_map;
	Socket::SocketList		m_ssl_sock_list;
	Socket::SocketList		m_reg_sock_list;
	Mutex					m_ssl_queue_mutex;
	Mutex					m_reg_queue_mutex;
	Mutex					m_request_queue_mutex;
	TaskManager*			m_ssl_task_manager;
	ThreadPool*				m_ssl_thread_pool;
	TaskManager*			m_reg_task_manager;
	ThreadPool*				m_reg_thread_pool;
};
#endif

