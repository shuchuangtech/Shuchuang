#include "Poco/Util/Timer.h"
#include "Poco/AutoPtr.h"
#include "MyTask.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/Timespan.h"
#include "Poco/Thread.h"
#include "Common/PrintLog.h"
int main(int argc, char** argv)
{
	Poco::Util::Timer timer;
	Poco::AutoPtr<MyTask> task1 = new MyTask;
	Poco::AutoPtr<MyTask> task2 = new MyTask;
	task1->setId(0);
	task2->setId(1);
	Poco::DateTime dt1;
	Poco::Timespan ts1(10, 0);
	dt1 -= ts1;
	timer.schedule(task1, dt1.timestamp(), 5 * 1000);
	tracef("%s, %d: Main call timer->schedule( 0 ).", __FILE__, __LINE__);
	Poco::Thread::sleep(3 * 1000);
	Poco::DateTime dt2;
	Poco::Timespan ts2(8, 0);
	dt2 += ts2;
	timer.schedule(task2, dt2.timestamp(), 7 * 1000);
	tracef("%s, %d: Main call timer->schedule( 1 ).", __FILE__, __LINE__);
	Poco::Thread::sleep(60 * 1000);

	task1->cancel();
	tracef("%s, %d: task1 cancelled.", __FILE__, __LINE__);
	Poco::Thread::sleep(5 * 1000);
	task2->cancel();
	tracef("%s, %d: task2 cancelled.", __FILE__, __LINE__);
	Poco::Thread::sleep(10 * 1000);

	Poco::DateTime dt3;
	Poco::Timespan ts3(4, 0);
	dt3 += ts3;
	try
	{
		timer.schedule(task1, dt3.timestamp(), 4 * 1000);
	}
	catch(Poco::Exception& e)
	{
		warnf("%s, %d: Eception: %s.", __FILE__, __LINE__, e.message().c_str());
		return -1;
	}
	tracef("%s, %d: Main call timer->schedule( 0 ).", __FILE__, __LINE__);
	Poco::Thread::sleep(30 * 1000);
	task1->cancel();
	timer.cancel(true);
	tracef("%s, %d: timer cancelled.", __FILE__, __LINE__);
	Poco::Thread::sleep(5 * 1000);
	return 0;
}

