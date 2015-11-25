#include <stdio.h>
int main(int argc, char** argv)
{
	char** c = new char*[10];
	for(int i = 0; i < 10; i++)
	{
		c[i] = new char[16];
	}
	int i = 0;
	const char* ethname = "eth0";
	const char* hostname = "hosthj";
	snprintf(c[i++], 15, "%s", "udhcpc");
	snprintf(c[i++], 15, "%s", "-i");
	snprintf(c[i++], 15, "%s", ethname);
	snprintf(c[i++], 15, "%s", "-R");
	snprintf(c[i++], 15, "%s", "-b");
	snprintf(c[i++], 15, "%s", "-p");
	snprintf(c[i++], 15, "%s", "/var/run/udhcpc.pid");
	snprintf(c[i++], 15, "%s", "-h");
	snprintf(c[i++], 15, "%s", hostname);
	c[i++] = NULL;
	char* const* cc = c;
	for(int i = 0; cc[i] != NULL; i++)
		printf("%s\n", cc[i]);

	for(int i = 0; i < 10; i++)
		delete[] c[i];
	delete[] c;
}

