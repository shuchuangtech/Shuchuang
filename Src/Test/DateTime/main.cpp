#include "Poco/DateTime.h"
#include "Poco/Timezone.h"
#include "Poco/Timestamp.h"
#include "Poco/Types.h"
#include "Poco/Timespan.h"
#include <stdio.h>
using namespace Poco;
int main()
{
	Poco::Int64 ti = -1;
	Timestamp test(ti);
	DateTime dtest(test);
	printf("%02d-%02d %02d:%02d:%02d\n", dtest.month(), dtest.day(), dtest.hour(), dtest.minute(), dtest.second());
	Timestamp now;
	DateTime dt(now);
	printf("%02d-%02d %02d:%02d:%02d\n", dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
	Timespan ts = dtest - dt;
	int total = ts.totalSeconds();
	printf("%d\n", total);
	int diff = Timezone::utcOffset();
	dt.makeLocal(diff);
	printf("%02d-%02d %02d:%02d:%02d\n", dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
	return 0;
}

