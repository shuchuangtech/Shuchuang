#include <stdio.h>
#include <typeinfo>
#include "Poco/Types.h"
#include "Poco/Dynamic/Var.h"
int main(int argc, char** argv)
{
	Poco::UInt64 type_uint64 = 123456789;
	printf("%s\n", typeid(type_uint64).name());
	unsigned long long type_long_long = 123456789;
	printf("%s\n", typeid(type_long_long).name());
	Poco::Dynamic::Var var(type_uint64);
	printf("%s\n", var.type().name());
	unsigned long type_long = 123456789;
	printf("%s\n", typeid(type_long).name());
	long long l = 123456789;
	printf("%s\n", typeid(l).name());
	return 0;
}

