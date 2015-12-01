#include "Poco/Dynamic/Struct.h"
#include <stdio.h>
int main()
{
	Poco::DynamicStruct ds;
	printf("%s\n", ds.toString().c_str());
	return 0;
}

