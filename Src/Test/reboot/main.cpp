#include <unistd.h>
#include <sys/reboot.h>
int main()
{
	sync();
	return reboot(RB_AUTOBOOT);
}

