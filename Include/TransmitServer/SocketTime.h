#ifndef __SERVER_SOCKET_TIME_H__
#define __SERVER_SOCKET_TIME_H__
#include "Poco/Net/StreamSocket.h"
#include "Poco/Timestamp.h"
struct _SocketTime
{
	_SocketTime(const Poco::Net::StreamSocket& sock, const Poco::Timestamp& t)
		:socket(sock), time(t)
	{
	}
	Poco::Net::StreamSocket socket;
	Poco::Timestamp time;
};
typedef struct _SocketTime SocketTime;
#endif

