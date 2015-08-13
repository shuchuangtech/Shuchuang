#include "TransmitServer/HTTPSHandler.h"
#include "Common/PrintLog.h"
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
		char buf[1024] = {0, };
		char t_buf[1024] = {0, };
		int pos = 0;
		StreamSocket sOut(m_sn->sockOut);
		while(sOut.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
		{
			sOut.setReceiveTimeout(Timespan(10, 0));
			int ret = 0;
			try
			{
				ret = sOut.receiveBytes(t_buf, 1024);
			}
			catch(Exception& e)
			{
				warnf("%s, %d: HTTPS connection %s[%lu] receive timeout.", __FILE__, __LINE__, sOut.peerAddress().toString().c_str(), (UInt64)sOut.impl());
			}
			if(ret == 0)
				break;
			snprintf(buf + pos, 1023 - pos, "%s", t_buf);
			pos += ret;
			memset(t_buf, 0, 1024);
		}
		if(pos == 0)
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
			sIn.sendBytes(buf, 1024);
			m_result = true;
		}
	}
	else if(m_type == 1)
		//from in to out
	{	
		char buf[1024] = {0, };
		char t_buf[1024] = {0, };
		int pos = 0;
		StreamSocket sIn(m_sn->sockIn);
		while(sIn.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
		{
			int ret = sIn.receiveBytes(t_buf, 1024);
			if(ret == 0)
				break;
			snprintf(buf + pos, 1023 - pos, "%s", t_buf);
			pos += ret;
			memset(t_buf, 0, 1024);
		}
		sIn.close();
		StreamSocket sOut(m_sn->sockOut);
		sOut.sendBytes(buf, 1024);
		sOut.close();
		tracef("%s, %d: socket %lu closed.", __FILE__, __LINE__, (UInt64)sIn.impl());
		tracef("%s, %d: socket %lu closed.", __FILE__, __LINE__, (UInt64)sOut.impl());
	}
	else
	{
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

