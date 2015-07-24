#ifndef __SERVER_HTTPS_ACCEPTOR_H__
#define __SERVER_HTTPS_ACCEPTOR_H__
#include "TransmitServer/SocketNode.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Types.h"
#include "Poco/SingletonHolder.h"
#include "Poco/Timer.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Mutex.h"
#include "Poco/Semaphore.h"
#include "Poco/TaskManager.h"
#include "Poco/ThreadPool.h"
#include "Poco/TaskNotification.h"
#include <map>
using namespace Poco;
using namespace Poco::Net;
class CHTTPSAcceptor
{
public:
	static CHTTPSAcceptor* instance()
	{
		static SingletonHolder<CHTTPSAcceptor> sh;
		return sh.get();
	}
	CHTTPSAcceptor();
	~CHTTPSAcceptor();
	bool start();
	bool stop();
	bool setPort(UInt16 port, UInt16 http_port);
	void acceptor(Timer& timer);
	void pollerOut(Timer& timer);
	void pollerIn(Timer& timer);
	void handleTask(TaskFinishedNotification* pNf);
private:
	bool						m_started;
	TaskManager*				m_task_manager;
	ThreadPool*					m_thread_pool;
	UInt16						m_port;
	UInt16						m_http_port;
	Timer						m_acceptor;
	Timer						m_pollerOut;
	Timer						m_pollerIn;
	SecureServerSocket*			m_ssl_acceptor;
	Mutex						m_map_lock;
	Semaphore					m_sem;
	std::map<UInt64, SocketNode*>	m_sock_out_map;
	std::map<UInt64, SocketNode*>	m_sock_in_map;
	Socket::SocketList			m_sockOut_list;
	Socket::SocketList			m_sockIn_list;
};
#endif

