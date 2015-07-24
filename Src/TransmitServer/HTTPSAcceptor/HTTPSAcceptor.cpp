#include "TransmitServer/HTTPSAcceptor.h"
#include "TransmitServer/HTTPSHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Observer.h"
#include "Common/PrintLog.h"
CHTTPSAcceptor::CHTTPSAcceptor()
:m_sem(0, 1000)
{
	m_started = false;
	m_port = 0;
	m_ssl_acceptor = NULL;
}

CHTTPSAcceptor::~CHTTPSAcceptor()
{
}

bool CHTTPSAcceptor::start()
{
	if(m_started)
	{
		warnf("%s, %d: HTTPS acceptor has already started.\n", __FILE__, __LINE__);
		return false;
	}
	Context::Ptr pContext = new Context(Context::TLSV1_SERVER_USE,
									"./privkey.pem",
									"./cert.pem",
									"",
									Context::VERIFY_NONE);
	if(m_ssl_acceptor != NULL)
		delete m_ssl_acceptor;
	m_ssl_acceptor = new SecureServerSocket(m_port, 64, pContext);
	m_thread_pool = new ThreadPool("HTTPSServer");
	m_task_manager = new TaskManager(*m_thread_pool);
	Observer<CHTTPSAcceptor, TaskFinishedNotification> observer(*this, &CHTTPSAcceptor::handleTask);
	m_task_manager->addObserver(observer);
	m_started = true;
	TimerCallback<CHTTPSAcceptor> acceptor(*this, &CHTTPSAcceptor::acceptor);
	m_acceptor.start(acceptor);
	TimerCallback<CHTTPSAcceptor> pollerOut(*this, &CHTTPSAcceptor::pollerOut);
	m_pollerOut.start(pollerOut);
	TimerCallback<CHTTPSAcceptor> pollerIn(*this, &CHTTPSAcceptor::pollerIn);
	m_pollerIn.start(pollerIn);
	infof("%s, %d: HTTPS acceptor start successfully.\n", __FILE__, __LINE__);
	return true;
}

bool CHTTPSAcceptor::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: HTTPS acceptor not started.\n", __FILE__, __LINE__);
		return false;
	}
	m_started = false;
	m_acceptor.stop();
	m_pollerOut.stop();
	m_pollerIn.stop();
	infof("%s, %d: HTTPS acceptor stop successfully.\n", __FILE__, __LINE__);
	return true;
}

bool CHTTPSAcceptor::setPort(UInt16 port, UInt16 http_port)
{
	if(port >= 65535 || port <= 0 || http_port >= 65535 || http_port <= 0)
		return false;
	m_port = port;
	m_http_port = http_port;
	return true;
}

void CHTTPSAcceptor::acceptor(Timer& timer)
{
	while(m_started)
	{
		if(m_ssl_acceptor == NULL)
		{
			Thread::sleep(500);
			continue;
		}
		SocketAddress clientAddress;
		StreamSocket sOut;
		while(true)
		{
			if(m_ssl_acceptor->poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
			{
				sOut = m_ssl_acceptor->acceptConnection(clientAddress);
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
		infof("%s, %d: https acceptor accept %s\n", __FILE__, __LINE__, clientAddress.toString().c_str());
		SocketNode* sn = new SocketNode();
		sn->sockOut = sOut;
		m_sock_out_map.insert(std::make_pair<UInt64, SocketNode*>((UInt64)sOut.impl(), sn));
		m_sockOut_list.push_back(sOut);
	}
}

void CHTTPSAcceptor::pollerIn(Timer& timer)
{
	Socket::SocketList readList;
	Socket::SocketList errorList;
	Socket::SocketList writeList;
	while(m_started)
	{
		if(m_sockIn_list.empty())
		{
			Thread::sleep(500);
			continue;
		}
		writeList.clear();
		errorList.clear();
		readList.clear();
		std::copy(m_sockIn_list.begin(), m_sockIn_list.end(), std::back_inserter(readList));
		if(Socket::select(readList, writeList, errorList, Timespan(5, 0)) > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				std::map<UInt64, SocketNode*>::iterator it_sock = m_sock_in_map.find((UInt64)((*it).impl()));
				if(it_sock == m_sock_in_map.end())
				{
					errorf("%s, %d: Unsupposed to be here.\n", __FILE__, __LINE__);
				}
				CHTTPSHandler* handler = new CHTTPSHandler();
				handler->setParam(1, it_sock->second, m_http_port);
				m_sock_in_map.erase(it_sock);
				Socket::SocketList::iterator t_it = find(m_sockIn_list.begin(), m_sockIn_list.end(), *it);
				if(t_it != m_sockIn_list.end())
				{
					m_sockIn_list.erase(t_it);
				}
				m_task_manager->start(handler);
			}
		}
	}
}

void CHTTPSAcceptor::pollerOut(Timer& timer)
{
	Socket::SocketList readList;
	Socket::SocketList errorList;
	Socket::SocketList writeList;
	while(m_started)
	{
		if(m_sockOut_list.empty())
		{
			Thread::sleep(500);
			continue;
		}
		writeList.clear();
		errorList.clear();
		readList.clear();
		std::copy(m_sockOut_list.begin(), m_sockOut_list.end(), std::back_inserter(readList));
		if(Socket::select(readList, writeList, errorList, Timespan(5, 0)) > 0)
		{
			for(Socket::SocketList::iterator it = readList.begin(); it != readList.end(); it++)
			{
				tracepoint();
				std::map<UInt64, SocketNode*>::iterator it_sock = m_sock_out_map.find((UInt64)((*it).impl()));
				if(it_sock == m_sock_out_map.end())
				{
					errorf("%s, %d: Unsupposed to be here.\n", __FILE__, __LINE__);
				}
				CHTTPSHandler* handler = new CHTTPSHandler();
				handler->setParam(0, it_sock->second, m_http_port);
				m_sock_out_map.erase(it_sock);
				Socket::SocketList::iterator t_it = find(m_sockOut_list.begin(), m_sockOut_list.end(), *it);
				if(t_it != m_sockOut_list.end())
				{
					m_sockOut_list.erase(t_it);
				}
				m_task_manager->start(handler);
			}
		}
	}
}

void CHTTPSAcceptor::handleTask(TaskFinishedNotification* pNf)
{
	AutoPtr<TaskNotification> p(pNf);
	if(p)
	{
		CHTTPSHandler* handler = (CHTTPSHandler*)p->task();
		if(handler->getType() == 0)
		{
			SocketNode* sn = handler->getSocketNode();
			if(!handler->getResult())
			{
				sn->sockOut.close();
				if(sn != NULL)
					delete sn;
			}
			else
			{
				StreamSocket sIn(sn->sockIn);
				m_sockIn_list.push_back(sIn);
				m_sock_in_map.insert(std::make_pair<UInt64, SocketNode*>((UInt64)(sIn.impl()), sn));
			}
		}
		else
		{
			SocketNode* sn = handler->getSocketNode();
			delete sn;
		}
	}
}

