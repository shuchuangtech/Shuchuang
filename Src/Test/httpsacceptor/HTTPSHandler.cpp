#include "TransmitServer/HTTPSHandler.h"
#include "Common/PrintLog.h"
CHTTPSHandler::CHTTPSHandler()
:Task("HTTPSHandler")
{
}

bool CHTTPSHandler::setParam(int type, SocketNode* sn)
{
	m_type = type;
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
			int ret = sOut.receiveBytes(t_buf, 1024);
			if(ret == 0)
				break;
			snprintf(buf + pos, 1023 - pos, "%s", t_buf);
			pos += ret;
			memset(t_buf, 0, 1024);
		}
		StreamSocket sIn(m_sn->sockIn);
		sIn.sendBytes(buf, 1024);
	}
	else if(m_type == 1)
		//from in to out
	{	
		char buf[1024] = {0, };
		char t_buf[1024] = {0, };
		int pos = 0;
		StreamSocket sIn(m_sn->sockIn);
		tracepoint();
		while(sIn.poll(Timespan(0, 100), Socket::SELECT_READ) > 0)
		{
			int ret = sIn.receiveBytes(t_buf, 1024);
			if(ret == 0)
				break;
			snprintf(buf + pos, 1023 - pos, "%s", t_buf);
			pos += ret;
			memset(t_buf, 0, 1024);
		}
		tracef("%s, %d: recv form in: %s\n", __FILE__, __LINE__, buf);
		StreamSocket sOut(m_sn->sockOut);
		sOut.sendBytes(buf, 1024);
		tracepoint();
	}
	else
	{
	}
}

int CHTTPSHandler::getType()
{
	return m_type;
}

int CHTTPSHandler::getOut()
{
	return (int)(m_sn->sockOut.impl());
}

int CHTTPSHandler::getIn()
{
	return (int)(m_sn->sockIn.impl());
}

