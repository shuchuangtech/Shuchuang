#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
static void SCSigSegvHandler(int signum)
{
	void* array[10];
	size_t size;
	char** strings;
	size_t i;

	signal(signum, SIG_DFL);
	size = backtrace(array, 10);
	strings = (char **)backtrace_symbols(array, size);
	fprintf(stderr, "Received SIGSEGV, Stack trace:\n");
	for(i = 2; i < size; i++)
	{
		fprintf(stderr, "%d %s\n", i - 2, strings[i]);
	}
	free(strings);
	exit(1);
}

void registerSignalHandler()
{
	signal(SIGSEGV, SCSigSegvHandler);
	signal(SIGABRT, SCSigSegvHandler);
}

