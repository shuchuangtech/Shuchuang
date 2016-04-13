#include <iostream>
#include <fstream>
#include <string.h>
int main()
{
	std::ifstream ifs("/home/huang_jian/Dev_Env/user_pass", std::ios::in);
	std::ofstream uuidOfs("/home/huang_jian/Dev_Env/username.data", std::ios::out);
	std::ofstream passwordOfs("/home/huang_jian/Dev_Env/password.data", std::ios::out);
	std::ofstream macOfs("/home/huang_jian/Dev_Env/mac.data", std::ios::out);
	char buf[64] = {0, };
	unsigned long first = 0x04766e000001;
	while (true) {
		ifs.getline(buf, 64);
		char uuid[16] = {0, };
		char username[16] = {0, };
		char password[16] = {0, };
		char macStr[32] = {0, };
		unsigned char mac[6];
		mac[0] = 0x0000000000ff & first;
		mac[1] = (0x00000000ff00 & first) >> 8;
		mac[2] = (0x000000ff0000 & first) >> 16;
		mac[3] = (0x0000ff000000 & first) >> 24;
		mac[4] = (0x00ff00000000 & first) >> 32;
		mac[5] = (0xff0000000000 & first) >> 40;
		snprintf(macStr, 32, "%02x-%02x-%02x-%02x-%02x-%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
		sscanf(buf, "%s\t%s\t%s", uuid, username, password);
		if (ifs.eof()) {
			break;
		}
		uuidOfs << uuid << std::endl;
		passwordOfs << password << std::endl;
		macOfs << macStr << std::endl;
		first++;
		printf("%s\t%s\t%s\t\n", uuid, password, macStr);
		memset(buf, 0, 64);
	}
	ifs.close();
	uuidOfs.close();
	passwordOfs.close();
	macOfs.close();
	return 0;
}
