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
		return false;
	
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
	TimerCallback<CHTTPSAcceptor> acceptor(*this, &CHTTPSAcceptor::acceptor);
	m_acceptor.start(acceptor);
	TimerCallback<CHTTPSAcceptor> pollerOut(*this, &CHTTPSAcceptor::pollerOut);
	m_pollerOut.start(pollerOut);
	//TimerCallback<CHTTPSAcceptor> pollerIn(*this, &CHTTPSAcceptor::pollerIn);
	//m_pollerIn.start(pollerIn);
	return true;
}

bool CHTTPSAcceptor::stop()
{
	if(!m_started)
		return false;
	m_acceptor.stop();
	m_pollerOut.stop();
	//m_pollerIn.stop();
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
	for(;;)
	{
		if(m_ssl_acceptor == NULL)
		{
			Thread::sleep(500);
			tracepoint();
			continue;
		}
		SocketAddress clientAddress;
		tracepoint();
		StreamSocket sOut;
		try
		{
			sOut = m_ssl_acceptor->acceptConnection(clientAddress);
		}
		catch(Exception& e)
		{
			tracef("%s, %d: %s\n", __FILE__, __LINE__, e.message().c_str());
		}
		infof("%s, %d: https acceptor accept %s\n", __FILE__, __LINE__, clientAddress.toString().c_str());
		//SocketAddress addrIn("127.0.0.1", m_http_port);
		//StreamSocket sIn(addrIn);
		//SocketNode* sn = new SocketNode(sOut, sIn);
		//m_sock_out_map.insert(std::make_pair<int, SocketNode*>((int)sOut.impl(), sn));
		//m_sock_in_map.insert(std::make_pair<int, SocketNode*>((int)sIn.impl(), sn));
		m_sockOut_list.push_back(sOut);
		//m_sockIn_list.push_back(sIn);
	}
}

/*void CHTTPSAcceptor::pollerIn(Timer& timer)
{
	Socket::SocketList readList;
	Socket::SocketList errorList;
	Socket::SocketList writeList;
	for(;;)
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
				std::map<int, SocketNode*>::iterator it_sock = m_sock_in_map.find((int)((*it).impl()));
				if(it_sock == m_sock_in_map.end())
				{
					errorf("%s, %d: Unsupposed to be here.\n", __FILE__, __LINE__);
				}
				//CHTTPSHandler* handler = new CHTTPSHandler();
				//handler->setParam(1, it_sock->second);
				StreamSocket sIn(*it);
				char buf[1024] = {0, };
				char t_buf[1024] = {0, };
				int pos = 0;
				while(sIn.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
				{
					tracepoint();
					int ret = sIn.receiveBytes(t_buf, 1024);
					if(ret == 0)
						break;
					snprintf(buf + pos, 1023 - pos, "%s", t_buf);
					pos += ret;
					//tracef("%s, %d: receive bytes: %s\n", __FILE__, __LINE__, t_buf);
					memset(t_buf, 0, 1024);
				}
				tracef("%s, %d: buf:%s\n", __FILE__, __LINE__, buf);
				Socket::SocketList::iterator t_it = find(m_sockIn_list.begin(), m_sockIn_list.end(), *it);
				if(t_it != m_sockIn_list.end())
				{
					m_sockIn_list.erase(t_it);
				}
				it_sock->second->sockOut.sendBytes(buf, 1024);
				
				//m_task_manager->start(handler);
			}
		}
	}
}*/

void CHTTPSAcceptor::pollerOut(Timer& timer)
{
	Socket::SocketList readList;
	Socket::SocketList errorList;
	Socket::SocketList writeList;
	for(;;)
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
				//std::map<int, SocketNode*>::iterator it_sock = m_sock_out_map.find((int)((*it).impl()));
				//if(it_sock == m_sock_out_map.end())
			//	{
			//		errorf("%s, %d: Unsupposed to be here.\n", __FILE__, __LINE__);
			//	}
				//CHTTPSHandler* handler = new CHTTPSHandler();
				//handler->setParam(0, it_sock->second);
				tracepoint();
				StreamSocket sOut(*it);
				char buf[1024] = {0, };
				int ret = sOut.receiveBytes(buf, 1024);
				tracef("%s, %d: ret %d, buf %s\n", __FILE__, __LINE__, ret, buf);
				SocketAddress addrIn("127.0.0.1", m_http_port);
				StreamSocket sIn(addrIn);
				memset(buf, 0, 1024);
				char t_buf[1024] = {0, };
				int pos = 0;
				while(sOut.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
				{
					tracepoint();
					int ret = sOut.receiveBytes(t_buf, 1024);
					if(ret == 0)
						break;
					snprintf(buf + pos, 1023 - pos, "%s", t_buf);
					pos += ret;
					//tracef("%s, %d: receive bytes: %s\n", __FILE__, __LINE__, t_buf);
					memset(t_buf, 0, 1024);
				}
				tracef("%s, %d: buf:%s\n", __FILE__, __LINE__, buf);
				sIn.sendBytes(buf, 1024);
				//m_sockIn_list.push_back(sIn);
				Socket::SocketList::iterator t_it = find(m_sockOut_list.begin(), m_sockOut_list.end(), *it);
				if(t_it != m_sockOut_list.end())
				{
					m_sockOut_list.erase(t_it);
				}
				if(sIn.poll(Timespan(5, 0), Socket::SELECT_READ) > 0)
				{
					memset(buf, 0, 1024);
					sIn.receiveBytes(buf, 1024);
					tracef("%s, %d: buf:%s\n", __FILE__, __LINE__, buf);
					sOut.sendBytes(buf, 1024);
					sIn.close();
					sOut.close();
				}
				//m_task_manager->start(handler);
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
		if(handler->getType() == 1)
		{
			int implOut = handler->getOut();
			int implIn = handler->getIn();
			std::map<int, SocketNode*>::iterator it = m_sock_out_map.find(implOut);
			if(it == m_sock_out_map.end())
			{
				errorf("%s, %d: Not supposed to be here.\n", __FILE__, __LINE__);
				return;
			}
			SocketNode* sn = it->second;
			sn->sockOut.close();
			sn->sockIn.close();
			delete sn;
			m_sock_out_map.erase(implOut);
			m_sock_in_map.erase(implIn);
		}
	}
}

