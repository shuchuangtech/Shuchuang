#ifndef __SERVER_SOCKET_NODE_H__
#define __SERVER_SOCKET_NODE_H__
#include "Poco/Net/StreamSocket.h"
struct _SocketNode
{
	_SocketNode()
	{
	}
	Poco::Net::StreamSocket sockOut;
	Poco::Net::StreamSocket sockIn;
};
typedef struct _SocketNode SocketNode;
#endif

