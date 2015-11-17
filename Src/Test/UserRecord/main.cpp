#include "Device/Util/UserRecord.h"
#include "Poco/Timestamp.h"
#include <stdio.h>
int main(int argc, char** argv)
{
	CUserRecord* ur = CUserRecord::instance();
	ur->init("/home/hj/Dev_Env/Shuchuang/test.db");
	int ret = 0;	
	UserRecordNode u1;
	u1.username = "huangjian";
	u1.password = "huangjian";
	u1.authority = 9;
	u1.timeOfValidity = -1;
	u1.remainOpen = -1;
	u1.token = "testtoken";
	Poco::Timestamp t;
	u1.lastVerify = t.epochMicroseconds();
	u1.lastLogin = t.epochMicroseconds();
	ret = ur->addUser(u1);
	printf("add u1 ret:%d\n", ret);
	ret = ur->getUser(u1);
	printf("get u1 ret:%d\n", ret);
	u1.password = "newpassword";
	ret = ur->updateUser(u1);
	printf("update u1 ret:%d\n", ret);
	UserRecordNode u2;
	u2.username = "linshu";	
	u2.password = "hj";
	u2.authority = 9;
	u2.timeOfValidity = -1;
	u2.remainOpen = -1;
	u2.token = "newtoken";
	u2.lastVerify = 1234567;
	u2.lastLogin = 1234567;
	ret = ur->getUser(u2);
	printf("get u2 ret:%d\n", ret);
	ret = ur->updateUser(u2);
	printf("update u2 ret:%d\n", ret);
	ret = ur->deleteUser(u1);
	printf("delete u1 ret:%d\n", ret);
	ret = ur->deleteUser(u2);
	printf("delete u2 ret:%d\n", ret);
//	ret = ur->updateUser(u2);

	/*
	ur->getUser(u2);
	u2.password = "newpassword";
	ur->updateUser(u2);
	UserRecordNode u3;
	u3.username = "huangjian";
	ur->getUser(u3);
	
	printf("get user \"huangjian\"\n");
	printf("password:%s\n", u3.password.c_str());
	printf("authority:%d\n", u3.authority);
	printf("timeOfValidity:%lld\n", u3.timeOfValidity);
	printf("remainOpen:%d\n", u3.remainOpen);
	printf("token:%s\n", u3.token.c_str());
	printf("lastVerify:%lld\n", u3.lastVerify);
	printf("lastLogin:%lld\n", u3.lastLogin);
	*/
	return 0;
}

