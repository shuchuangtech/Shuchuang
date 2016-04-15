#include "Poco/DateTime.h"
#include "Poco/Timezone.h"
#include "Poco/Net/NTPClient.h"
#include "Poco/Net/NTPEventArgs.h"
#include "Poco/Net/NTPPacket.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Delegate.h"
#include "Poco/Thread.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
class Target
{
public:
	void onEvent(const void* pSender, Poco::Net::NTPEventArgs& arg)
	{
		printf("receive.\n");
		Poco::Net::NTPPacket packet = arg.packet();
		Poco::DateTime t1(packet.originateTime());
		printf("T1: %04d-%02d-%02d %02d:%02d:%02d.\n", t1.year(), t1.month(), t1.day(), t1.hour(), t1.minute(), t1.second());
		Poco::DateTime t4(packet.receiveTime());
		printf("T4: %04d-%02d-%02d %02d:%02d:%02d.\n", t4.year(), t4.month(), t4.day(), t4.hour(), t4.minute(), t4.second());
		printf("root delay %d, root dispersion %d.\n", packet.rootDelay(), packet.rootDispersion());
	}
};
int main(int argc, char** argv)
{
	Poco::Net::NTPClient ntpc(Poco::Net::IPAddress::IPv4, 10 * 1000 * 1000);
	Target target;
	ntpc.response += Poco::delegate(&target, &Target::onEvent);
	char choice = 0;
	printf("1.time.asia.apple.com\n");
	printf("2.shuchuangtech.com\n");
	printf("3.133.100.11.8\n");
	printf("4.www.freebsd.com\n");
	printf("5.ntp.nasa.org\n");
	printf("6.ntp.sjtu.edu.cn\n");
	printf("7.else\n");
	scanf("%c", &choice);
	try
	{
		int result = 0;
		switch(choice)
		{
			case '1':
				result = ntpc.request("time.asia.apple.com");
				break;
			case'2':
				result = ntpc.request("shuchuangtech.com");
				break;
			case '3':
				result = ntpc.request("133.100.11.8");
				break;
			case '4':
				result = ntpc.request("www.freebsd.com");
				break;
			case '5':
				result = ntpc.request("ntp.nasa.org");
				break;
			case '6':
				result = ntpc.request("ntp.sjtu.edu.cn");
				break;
			case '7':
				char server[64];
				memset(server, 0, 64);
				printf("input server address:");
				scanf("%s", server);
				result = ntpc.request(server);
				break;
			printf("result %d\n", result);
		}
	}
	catch(Poco::Exception& e)
	{
		printf("exception %s.\n", e.message().c_str());
	}
	//Poco::Thread::sleep(1 * 1000);
	ntpc.response -= Poco::delegate(&target, &Target::onEvent);
	Poco::DateTime now;
	printf("before make local: %04d-%02d-%02d %02d:%02d:%02d.\n", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
	now.makeLocal(Poco::Timezone::utcOffset());
	printf("after make local: %04d-%02d-%02d %02d:%02d:%02d.\n", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
	//Poco::DateTime dt(now.year(), now.month(), now.day(),);
	return 0;
}

