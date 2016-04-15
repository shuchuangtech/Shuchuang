#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "Poco/DateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/Timezone.h"
int SetSystemTime(Poco::DateTime dt)
{
	Poco::Timestamp ts = dt.timestamp();
	struct timeval tv;
	tv.tv_sec = (ts.epochMicroseconds() / (1000 * 1000));
	tv.tv_usec = 0;
	printf("%ld, %ld\n", tv.tv_sec, tv.tv_usec);
	struct timezone tz;
	struct timeval gtv;
	if(gettimeofday(&gtv, &tz) == 0)
	{
		printf("gettimeofday sec %ld, usec %ld, minutewest %d, dst %d\n", gtv.tv_sec, gtv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
	}
	if(settimeofday(&tv, &tz) < 0)
	{
		printf("set system datetime error, %d\n", errno);
		if(errno == EFAULT)
			printf("EFAULT\n");
		else if(errno == EINVAL)
			printf("EINVAL\n");
		else if(errno == EPERM)
			printf("EPERM\n");
		return -1;
	}
	dt.makeLocal(Poco::Timezone::utcOffset());
	printf("%04d-%02d-%02d %02d:%02d:%02d\n", dt.year(), dt.month(),dt.day(), dt.hour(), dt.minute(), dt.second());
	return 0;
}

int main()
{
	Poco::DateTime dt(2016, 4, 14, 12, 0, 0);
	if(putenv("TZ=WAUST-8WAUDT") != 0)
	{
		printf("putenv failed, %d\n", errno);
	}
	tzset();
	SetSystemTime(dt);
	return 0;
}
