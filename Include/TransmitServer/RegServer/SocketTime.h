#ifndef __SERVER_SOCKET_TIME_H__
#define __SERVER_SOCKET_TIME_H__
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Timestamp.h"
struct _SocketTime
{
	enum
	{
		Disconnected = 0,
		Connecting,
		Connected
	};
	_SocketTime(const Poco::Net::StreamSocket& sock, const Poco::Timestamp& t)
		:socket(sock), time(t), state(Disconnected)
	{
		saddr = sock.peerAddress();
	}
	Poco::Net::StreamSocket socket;
	Poco::Net::SocketAddress saddr;
	Poco::Timestamp time;
	int state;
};
typedef struct _SocketTime SocketTime;
#endif

