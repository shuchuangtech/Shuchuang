#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/DynamicStruct.h"
#include "Poco/JSON/Array.h"
#include "Poco/Timestamp.h"
#include <stdio.h>
#include "Poco/Thread.h"
using namespace Poco;
using namespace Poco::Net;
int main(int argc, char** argv)
{
	
	SocketAddress sa("127.0.0.1", 12222);
	StreamSocket sock(sa);
	
	JSON::Array::Ptr pArray = new JSON::Array;
	for(int i = 0; i < 100; i++)
	{
		DynamicStruct ds;
		Timestamp now;
		ds["id"] = i;
		ds["operation"] = "close";
		ds["timestamp"] = now.epochMicroseconds();
		pArray->add(ds);
	}
	DynamicStruct ds;
	ds["record"] = pArray;
	//printf("length:%d, content:\n%s\n", ds.toString().length(), ds.toString().c_str());
	Thread::sleep(5000);
	int ret = sock.sendBytes(ds.toString().c_str(), ds.toString().length());
	printf("ret %d\n", ret);
	for(int i = 0; i < 10; i++)
	{
		printf("sleep %d...\n", i);
		Thread::sleep(1000);
	}
	return 0;
}

