#include "Poco/Timezone.h"
#include <stdio.h>
int main()
{
	printf("utcOffset %d, tzd %d", Poco::Timezone::utcOffset(), Poco::Timezone::tzd());
	return 0;
}

