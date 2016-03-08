#include "Poco/Net/FTPClientSession.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
int main()
{
	//Poco::Net::FTPClientSession ftp("shuchuangtech.com", 21, "shuchuangftp", "drmfSlxd12");
	Poco::Net::FTPClientSession ftp("shuchuangtech.com", 21, "sc", "SCDevicePublic123");
	try {
		std::istream& ist = ftp.beginDownload("NoneExistFile");
		std::ofstream ofs("./noneExist", std::ios::out|std::ios::trunc|std::ios::binary);
		ofs << ist.rdbuf();
		ofs.close();
		ftp.endDownload();
	}
	catch(Poco::Exception& e) {
		printf("%s\n", e.message().c_str());
		return 0;
	}

	return 0;
}

