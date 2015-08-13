#ifndef __SERVER_SOCKET_NODE_H__
#define __SERVER_SOCKET_NODE_H__
#include "Poco/Net/StreamSocket.h"
#include "TransmitServer/SocketNode.h"
using namespace Poco::Net;
struct _SocketNode
{
	_SocketNode()
	{
	}
	StreamSocket sockOut;
	StreamSocket sockIn;
};
typedef struct _SocketNode SocketNode;
#endif

