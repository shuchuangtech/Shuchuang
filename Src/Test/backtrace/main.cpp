#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
static void WidebrightSegvHandler(int signum)
{
	void* array[10];
	size_t size;
	char** strings;
	size_t i;

	signal(signum, SIG_DFL);
	size = backtrace(array, 10);
	fprintf(stderr, "size %d\n", size);
	strings = (char **)backtrace_symbols(array, size);
	fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");
	for(i = 0; i < size; i++)
	{
		fprintf(stderr, "%d %s\n", i, strings[i]);
	}
	free(strings);
	exit(1);
}

int invalid_pointer_error(char* p)
{
	*p = 'd';
	return 0;
}

void error_2(char* p)
{
	invalid_pointer_error(p);
}

void error_1(char* p)
{
	error_2(p);
}

void error_0(char* p)
{
	error_1(p);
}

int main()
{
	signal(SIGSEGV, WidebrightSegvHandler);
	signal(SIGABRT, WidebrightSegvHandler);

	char* a = NULL;
	error_0(a);
	return 0;
}

