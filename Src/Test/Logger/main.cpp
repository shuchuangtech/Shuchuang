#include "PrintLog.h"
#include "Poco/Thread.h"
#include "stdio.h"
using namespace Poco;
int main()
{
	initPrintLogger();
	for(int i = 0; i < 100; i++)
	{
		tracef("%s, %d: Test information %d", __FILE__, __LINE__, i);
		Thread::sleep(100);
	}
	return 0;
}

