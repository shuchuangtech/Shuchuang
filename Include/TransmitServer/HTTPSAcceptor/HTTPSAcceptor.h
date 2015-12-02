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
class CHTTPSAcceptor
{
public:
	static CHTTPSAcceptor* instance()
	{
		static Poco::SingletonHolder<CHTTPSAcceptor> sh;
		return sh.get();
	}
	CHTTPSAcceptor();
	~CHTTPSAcceptor();
	bool start();
	bool stop();
	bool setPort(Poco::UInt16 port, Poco::UInt16 http_port);
	void acceptor(Poco::Timer& timer);
	void pollerOut(Poco::Timer& timer);
	void pollerIn(Poco::Timer& timer);
	void handleTask(Poco::TaskFinishedNotification* pNf);
private:
	bool									m_started;
	Poco::TaskManager*						m_task_manager;
	Poco::ThreadPool*						m_thread_pool;
	Poco::UInt16							m_port;
	Poco::UInt16							m_http_port;
	Poco::Timer								m_acceptor;
	Poco::Timer								m_pollerOut;
	Poco::Timer								m_pollerIn;
	Poco::Net::SecureServerSocket*			m_ssl_acceptor;
	Poco::Mutex								m_map_lock;
	std::map<Poco::UInt64, SocketNode*>		m_sock_out_map;
	std::map<Poco::UInt64, SocketNode*>		m_sock_in_map;
	Poco::Net::Socket::SocketList			m_sockOut_list;
	Poco::Net::Socket::SocketList			m_sockIn_list;
};
#endif

