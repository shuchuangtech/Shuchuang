#include "Poco/Util/JSONConfiguration.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/Session.h"
#include "Poco/SHA1Engine.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/Dynamic/VarHolder.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/AutoPtr.h"
#include "Poco/FileStream.h"
#include "Poco/File.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace Poco;
void initDefaultConfig(JSON::Object::Ptr& pRoot, int deviceOrServer, std::string uuid, std::string mac)
{
	JSON::Object::Ptr pNode = new JSON::Object;
	if(deviceOrServer == 0)
	{
		//DeviceInfo
		JSON::Object::Ptr pDeviceInfo = new JSON::Object;
		pDeviceInfo->set("uuid", uuid);
		pDeviceInfo->set("type", "sc-lock0001");
		pDeviceInfo->set("name", uuid);
		pDeviceInfo->set("manufacture", "Shuchuangtech");
		pDeviceInfo->set("mac", mac);
		//RegProxy
		JSON::Object::Ptr pRegProxy = new JSON::Object;
		pRegProxy->set("host", "shuchuangtech.com");
		pRegProxy->set("ssl_port", 12222);
		pRegProxy->set("reg_port", 13333);
		//APNS
		JSON::Object::Ptr pAPNS = new JSON::Object;
		pAPNS->set("MobileToken", "");
		pAPNS->set("InstallationId", "");
		//Tasks
		JSON::Array::Ptr pTasks = new JSON::Array;
		//DataPath
		JSON::Object::Ptr pDataPath = new JSON::Object;
		pDataPath->set("User", "/mnt/nand1-2/Application/user.db");
		pDataPath->set("Operation", "/mnt/nand1-2/Application/oprecord.db");
		//JSON::Object::Ptr nil = NULL;
		JSON::Object::Ptr pReset = new JSON::Object;
		pReset->set("UserDB", "/mnt/nand1-2/Application/backup/user.db");
		pReset->set("Config", "/mnt/nand1-2/Application/backup/global.conf");
		//Update
		JSON::Object::Ptr pUpdate = new JSON::Object;
		pUpdate->set("server", "shuchuangtech.com");
		pUpdate->set("username", "sc");
		pUpdate->set("password", "SCDevicePublic123");
		pUpdate->set("appPath", "/mnt/nand1-2/Application/DeviceTest");
		pUpdate->set("infoPath", "/mnt/nand1-2/Application/Version.info");
		
		//UserMode
		JSON::Object::Ptr pMode = new JSON::Object;
		pMode->set("mode", 0);

		//pTasks->add(nil);
		pNode->set("APNS", pAPNS);
		pNode->set("Tasks", pTasks);
		pNode->set("DeviceInfo", pDeviceInfo);
		pNode->set("RegProxy", pRegProxy);
		pNode->set("DataPath", pDataPath);
		pNode->set("Reset", pReset);
		pNode->set("Update", pUpdate);
		pNode->set("UserMode", pMode);
	}
	else if(deviceOrServer == 1)
	{
		JSON::Object::Ptr pRegServer = new JSON::Object;
		pRegServer->set("ssl_port", 12222);
		pRegServer->set("reg_port", 13333);

		JSON::Object::Ptr pHTTPServer = new JSON::Object;
		pHTTPServer->set("port", 8777);
		pHTTPServer->set("port", 9888);
		pHTTPServer->set("cert", "./cert.pem");
		pHTTPServer->set("privkey", "./privkey.pem");
		
		JSON::Object::Ptr pUpdate = new JSON::Object;
		pUpdate->set("DirPath", "./update/");
		
		JSON::Object::Ptr pAPNS = new JSON::Object;
		JSON::Array::Ptr pProxy = new JSON::Array;
		pProxy->add("bmob");
		pAPNS->set("Proxy", pProxy);
		JSON::Object::Ptr pBmob = new JSON::Object;
		pBmob->set("Host", "api.bmob.cn");
		pBmob->set("URI", "/1/push");
		pBmob->set("APPID", "27f1f3599a223cfa40bb5c5e5daedd7a");
		pBmob->set("APPKey", "0952518ed9dd101fab4c5c02d957f62d");
		pAPNS->set("bmob", pBmob);

		pNode->set("RegServer", pRegServer);
		pNode->set("HTTPServer", pHTTPServer);
		pNode->set("Update", pUpdate);
		pNode->set("APNS", pAPNS);
	}
	pRoot->set("root", pNode);
}

int writeBsFile(const char* fn, const char* uuid)
{
	std::ofstream fos(fn, std::ios::out);
	fos << "#!/bin/sh" << std::endl;
	fos << "echo \"Run boot_script\"" << std::endl;
	fos << "if [ ! -d \"/dev/pts\" ]; then" << std::endl;
	fos << "\tmkdir /dev/pts" << std::endl;
	fos << "\tmount -t devpts devpts /dev/pts" << std::endl;
	fos << "fi" << std::endl;
	fos << "cd /mnt/nand1-1/" << std::endl;
	fos << "insmod hzgpio.ko" << std::endl;
	fos << "rm /mnt/nand1-2/Application/log/*" << std::endl;
	fos << "/mnt/nand1-2/Application/netinitTest /mnt/nand1-2/Application/config" << std::endl;
	fos << "udhcpc -i eth0 -p /var/run/udhcpc.pid -R -b -h " << uuid << std::endl;
	fos << "if [ -f \"/mnt/nand1-2/Application/DeviceTest_temp\" ]; then" << std::endl;
	fos << "\tmv /mnt/nand1-2/Application/DeviceTest_temp /mnt/nand1-2/Application/DeviceTest" << std::endl;
	fos << "fi" <<std::endl;
	fos << "/mnt/nand1-2/Application/DeviceTest /mnt/nand1-2/Application/config /mnt/nand1-2/Application/log/device.log &" << std::endl;
	fos.close();
	return 0;
}

bool checkMakeDir(const char* dirPath)
{
	if(access(dirPath, F_OK) != 0) {
		if(mkdir(dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
			printf("目录 %s 创建失败\n", dirPath);
			return false;
		}
	}
	return true;
}

void generateDir(const char* outputDirPath, const char* uuid)
{
	//outputDirPath/uuid
	char dir[128] = {0, };
	snprintf(dir, 128, "%s/%s", outputDirPath, uuid);
	checkMakeDir(dir);
	//outputDirPath/uuid/nand1-1
	char nand1Dir[128];
	snprintf(nand1Dir, 128, "%s/nand1-1", dir);
	checkMakeDir(nand1Dir);
	//outputDirPath/uuid/nand1-2
	char nand2Dir[128];
	snprintf(nand2Dir, 128, "%s/nand1-2", dir);
	checkMakeDir(nand2Dir);
	//outputDirPath/uuid/nand1-2/Application
	char appDir[128];
	snprintf(appDir, 128, "%s/Application", nand2Dir);
	checkMakeDir(appDir);
	//outputDirPath/uuid/nand1-2/Application/config
	char configDir[128];
	snprintf(configDir, 128, "%s/config", appDir);
	checkMakeDir(configDir);
	//outputDirPath/uuid/nand1-2/Application/backup
	char backupDir[128];
	snprintf(backupDir, 128, "%s/backup", appDir);
	checkMakeDir(backupDir);	
}

void generateUserDB(const char* dbPath, const char* username, const char* password)
{
	Poco::Data::Session* session = new Poco::Data::Session("SQLite", dbPath);
	Poco::Data::Statement create(*session);
	create << "CREATE TABLE IF NOT EXISTS `User` ("
			"`Id` INTEGER PRIMARY KEY AUTOINCREMENT,"
			"`Username` VARCHAR(64) NOT NULL UNIQUE,"
			"`Password` VARCHAR(64) NOT NULL,"
			"`BindUser` VARCHAR(64),"
			"`Authority` TINYINT,"
			"`TimeOfValidity` BIGINT,"
			"`RemainOpen` INTEGER,"
			"`Token` VARCHAR(64),"
			"`LastVerify` BIGINT,"
			"`LastLogin` BIGINT)", Poco::Data::Keywords::now;

	Poco::SHA1Engine sha1;
	sha1.update(password);
	const Poco::DigestEngine::Digest& digest = sha1.digest();
	std::string sha1pass(Poco::DigestEngine::digestToHex(digest));
	Poco::DateTime tov(2050, 1, 1);
	long int timeOfValidity = tov.timestamp().epochMicroseconds();
	int authority = 9;
	int remainOpen = -1;
	std::string token = "";
	std::string binduser = "";
	int lastVerify = 0;
	int lastLogin = 0;
	Poco::Data::Statement insert(*session);
	insert << "INSERT INTO `User` (`Username`, `Password`, `BindUser`, `Authority`, `TimeOfValidity`, `RemainOpen`, `Token`, `LastVerify`, `LastLogin`) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)",
		   Poco::Data::Keywords::use(username),
		   Poco::Data::Keywords::use(sha1pass),
		   Poco::Data::Keywords::use(binduser),
		   Poco::Data::Keywords::use(authority),
		   Poco::Data::Keywords::use(timeOfValidity),
		   Poco::Data::Keywords::use(remainOpen),
		   Poco::Data::Keywords::use(token),
		   Poco::Data::Keywords::use(lastVerify),
		   Poco::Data::Keywords::use(lastLogin);
	try
	{
		insert.execute();
	}
	catch(Poco::Exception& e)
	{
		printf("%s\n", e.message().c_str());
	}

	session->close();
	delete session;

}

int main()
{
	int deviceOrServer = 0;
	printf("0. Generate device default config.\n"
			"1. Generate server default config.\n");
	int ret = scanf("%d", &deviceOrServer);
	if(ret < 0)
		return 0;
	
	printf("Output dir:");
	char outputDirPath[128];
	scanf("%s", outputDirPath);

	if(deviceOrServer == 0)
	{
		Poco::Data::SQLite::Connector::registerConnector();
		printf("Begin number:");
		int begin = 0;
		scanf("%d", &begin);
		printf("End number:");
		int end = 0;
		scanf("%d", &end);
		unsigned long first = 0x04766e000001;
		char buf[64];
		std::ifstream userdata("/home/huang_jian/Dev_Env/user_pass", std::ios::in);
		for(int j = 1; j < begin; j++)
		{
			userdata.getline(buf, 64);
		}
		for(int i = begin; i <= end; i++)
		{
			char uuid[13];
			snprintf(uuid, 13, "SC%010d", i);
			char dir[128];
			snprintf(dir, 128, "%s/%s", outputDirPath, uuid);
			printf("output device env dir:%s\n", dir);
			generateDir(outputDirPath, uuid);
			char cfgPath[128];
			char cfgBackupPath[128];
			char userDbBackupPath[128];
			char infoPath[128];
			char userDbPath[128];
			//boot_script
			char bsPath[128] = {0, };
			snprintf(bsPath, 128, "%s/nand1-1/boot_script", dir);
			writeBsFile(bsPath, uuid);
			//config
			snprintf(cfgPath, 128, "%s/nand1-2/Application/config/global.conf", dir);
			unsigned long mac_num = i - 1 + first;
			char mac_str[16] = {0, };
			snprintf(mac_str, 16, "%012lx", mac_num);
			JSON::Object::Ptr pObj = new JSON::Object;
			initDefaultConfig(pObj, 0, uuid, mac_str);
			FileOutputStream confFile(cfgPath);
			AutoPtr<Util::JSONConfiguration> conf = new Util::JSONConfiguration(pObj);
			conf->save(confFile);
			confFile.close();
			//userdb
			snprintf(userDbPath, 128, "%s/nand1-2/Application/user.db", dir);
			memset(buf, 0, 64);
			userdata.getline(buf, 64);
			printf("%s\n", buf);
			char uuid2[16] = {0, };
			char username[16] = {0, };
			char password[16] = {0, };
			sscanf(buf, "%s\t%s\t%s", uuid2, username, password);
			printf("%s, %s\n", username, password);
			generateUserDB(userDbPath, username, password);
			//version.info
			snprintf(infoPath, 128, "%s/nand1-2/Application/Version.info", dir);
			Poco::File infoFile("/home/huang_jian/Dev_Env/Shuchuang/Version.info");
			infoFile.copyTo(infoPath);
			//backup
			snprintf(cfgBackupPath, 128, "%s/nand1-2/Application/backup/global.conf", dir);
			Poco::File cfgFile(cfgPath);
			cfgFile.copyTo(cfgBackupPath);
			snprintf(userDbBackupPath, 128, "%s/nand1-2/Application/backup/user.db", dir);
			Poco::File userDbFile(userDbPath);
			userDbFile.copyTo(userDbBackupPath);
		}
		userdata.close();
	}
	else
	{
		JSON::Object::Ptr pObj = new JSON::Object;
		initDefaultConfig(pObj, 1, "", "");
		std::string outputFile = outputDirPath;
		outputFile += "/global.conf";
		FileOutputStream confFile(outputFile);
		AutoPtr<Util::JSONConfiguration> conf = new Util::JSONConfiguration(pObj);
		conf->save(confFile);
	}
	return 0;
}

