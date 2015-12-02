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
#include "TransmitServer/RegServer/RequestInfo.h"
#include "TransmitServer/RegServer/SocketTime.h"
#include "TransmitServer/OfflineNotification.h"
class CRegServer
{
public:
	static CRegServer* instance()
	{
		static Poco::SingletonHolder<CRegServer> sh;
		return sh.get();
	};
	CRegServer();
	~CRegServer();
	bool start();
	bool stop();
	bool setServerPort(Poco::UInt16 ssl_port, Poco::UInt16 reg_port);
	void sslAccept(Poco::Timer& timer);
	void sslHandler(Poco::Timer& timer);
	void regAccept(Poco::Timer& timer);
	void regHandler(Poco::Timer& timer);
	void httpRequestTimer(Poco::Timer& timer);
	void handleTaskFinish(Poco::TaskFinishedNotification* pNf);
	bool sendRequest(RequestInfo* request);
	void handleOffline(OfflineNotification* pNf);
private:
	bool removeSocket(int choice, std::map<Poco::UInt64, SocketTime*>::iterator it);
	void handleInnerSocket(Poco::Timer& timer);
	bool writeInnerSocket();
	bool readInnerSocket();
	bool createInnerSocket();
	//0 ssl, 1 reg
	bool								listenSsl();
	bool								listenReg();
	bool								m_started;
	Poco::UInt16						m_ssl_port;
	Poco::UInt16						m_reg_port;
	Poco::Timer							m_ssl_accept;
	Poco::Timer							m_reg_accept;
	Poco::Timer							m_ssl_handler;
	Poco::Timer							m_reg_handler;
	Poco::Timer							m_http_request_timer;
	Poco::Net::SecureServerSocket*		m_ssl_sock;
	Poco::Net::SecureServerSocket*		m_reg_sock;
	std::map<Poco::UInt64, SocketTime*>		m_pSsl_map;
	std::map<Poco::UInt64, SocketTime*>		m_pReg_map;
	std::map<Poco::UInt64, RequestInfo*>	m_request_map;
	Poco::Net::Socket::SocketList		m_ssl_sock_list;
	Poco::Net::Socket::SocketList		m_reg_sock_list;
	Poco::Mutex							m_ssl_queue_mutex;
	Poco::Mutex							m_reg_queue_mutex;
	Poco::Mutex							m_request_queue_mutex;
	Poco::TaskManager*					m_task_manager;
	Poco::ThreadPool*					m_thread_pool;
	//Inner socket
	Poco::Net::StreamSocket				m_inner_write_socket;
	Poco::Net::StreamSocket				m_inner_read_socket;
};
#endif

