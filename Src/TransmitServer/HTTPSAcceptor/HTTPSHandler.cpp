#include "TransmitServer/HTTPSAcceptor/HTTPSHandler.h"
#include "Common/PrintLog.h"
using namespace Poco;
using namespace Poco::Net;
CHTTPSHandler::CHTTPSHandler()
:Task("HTTPSHandler")
{
}

bool CHTTPSHandler::setParam(int type, SocketNode* sn, int http_port)
{
	if(type != 0 && type != 1)
		return false;
	m_type = type;
	if(http_port < 1 || http_port > 65534)
		return false;
	m_http_port = http_port;
	if(sn == NULL)
		return false;
	m_sn = sn;
	return true;
}

void CHTTPSHandler::runTask()
{
	if(m_type == 0)
		//from out to in
	{
		std::string buf = "";
		char t_buf[512] = {0, };
		StreamSocket sOut(m_sn->sockOut);
		sOut.setReceiveTimeout(Timespan(10, 0));
		try
		{
			if(sOut.receiveBytes(t_buf, 512) <= 0)
			{
				warnf("%s, %d: socket %s[%llu] receive error.", __FILE__, __LINE__, sOut.peerAddress().toString().c_str(), (UInt64)sOut.impl());
			}
			else
			{
				buf += t_buf;
				while(sOut.available())
				{
					memset(t_buf, 0, 512);
					if(sOut.receiveBytes(t_buf, 512) > 0)
					{
						buf += t_buf;
						tracepoint();
					}
				}
			}
		}
		catch(Exception& e)
		{
			warnf("%s, %d: HTTPS connection %s[%lu] receive timeout.", __FILE__, __LINE__, sOut.peerAddress().toString().c_str(), (UInt64)sOut.impl());
		}
		if(buf.empty())
			//receive nothing
			//sockOut disconnected
		{
			m_result = false;
		}
		else
		{
			SocketAddress sa("127.0.0.1", m_http_port);
			StreamSocket sIn(sa);
			tracef("%s, %d: socket %lu connect to http server.", __FILE__, __LINE__, (UInt64)sIn.impl());
			m_sn->sockIn = sIn;
			sIn.sendBytes(buf.c_str(), buf.length());
			m_result = true;
		}
	}
	else if(m_type == 1)
		//from in to out
	{	
		std::string buf = "";
		char t_buf[512] = {0, };
		StreamSocket sIn(m_sn->sockIn);
		sIn.setReceiveTimeout(Timespan(10, 0));
		try
		{
			if(sIn.receiveBytes(t_buf, 512) <= 0)
			{
				warnf("%s, %d: socket %s[%llu] receive error.", __FILE__, __LINE__, sIn.peerAddress().toString().c_str(), (UInt64)sIn.impl());
			}
			else
			{
				buf += t_buf;
				while(sIn.available())
				{
					memset(t_buf, 0, 512);
					if(sIn.receiveBytes(t_buf, 512) > 0)
					{
						buf += t_buf;
					}
				}
			}
		}
		catch(Exception& e)
		{
			warnf("%s, %d: HTTPS connection %s[%llu] receive timeout.", __FILE__, __LINE__, sIn.peerAddress().toString().c_str(), (UInt64)sIn.impl());
		}
		sIn.close();
		StreamSocket sOut(m_sn->sockOut);
		sOut.sendBytes(buf.c_str(), buf.length());
		sOut.close();
		tracef("%s, %d: socket %llu closed.", __FILE__, __LINE__, (UInt64)sIn.impl());
		tracef("%s, %d: socket %llu closed.", __FILE__, __LINE__, (UInt64)sOut.impl());
	}
	else
	{
		warnf("%s, %d: HTTPSHandler deal with unkown type.", __FILE__, __LINE__);
	}
}

int CHTTPSHandler::getType()
{
	return m_type;
}

SocketNode* CHTTPSHandler::getSocketNode()
{
	return m_sn;
}

bool CHTTPSHandler::getResult()
{
	return m_result;
}

