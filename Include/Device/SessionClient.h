#ifndef __DEVICE_SESSION_CLIENT_H__
#define __DEVICE_SESSION_CLIENT_H__
#include "Device/RPCServer.h"
#include "Poco/Types.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Common/RequestNotification.h"
#include "Poco/Timestamp.h"
using namespace Poco;
using namespace Poco::Net;
class CSessionClient : Runnable
{
public:
	CSessionClient();
	~CSessionClient();
	bool connect(std::string host, UInt16 port, UInt64 timeout);
	bool disconnect();
	int getID();
	void run();
	void handleNf(RequestNotification*);
	SocketAddress getAddress();
	bool checkState();

private:
	void dealError();
	Thread			m_thread;
	bool			m_connected;
	StreamSocket*	m_sock;
	int				m_id;
	CRPCServer*		m_rpc;
	Timestamp		m_lastCheckTime;
	Timestamp::TimeDiff	m_keepAliveTimeout;
};
#endif

