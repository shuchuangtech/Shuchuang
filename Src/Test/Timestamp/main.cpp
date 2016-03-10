#include "Poco/Timestamp.h"
#include <stdio.h>
int main()
{
	Poco::Timestamp now;
	printf("now timestamp:%ld\n", now.epochMicroseconds());
	return 0;
}

