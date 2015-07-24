#ifndef __SERVER_SOCKET_NODE_H__
#define __SERVER_SOCKET_NODE_H__
#include "Poco/Net/StreamSocket.h"
using namespace Poco::Net;
struct _SocketNode
{
	StreamSocket sockOut;
	StreamSocket sockIn;
};
typedef struct _SocketNode SocketNode;
#endif

