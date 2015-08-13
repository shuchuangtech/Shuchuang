#include "Poco/DateTime.h"
#include "Poco/Timezone.h"
#include "Poco/Timestamp.h"
#include <stdio.h>
using namespace Poco;
int main()
{
	Timestamp now;
	DateTime dt(now);
	printf("%02d-%02d %02d:%02d:%02d\n", dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
	int diff = Timezone::utcOffset();
	dt.makeLocal(diff);
	printf("%02d-%02d %02d:%02d:%02d\n", dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
	return 0;
}

