#include "Device/Util/UserRecord.h"
#include "Poco/Timestamp.h"
#include <stdio.h>
CUserRecord* ur = NULL;
UserRecordNode user[6] = {{"huangjian", "hjpass", 9, -1, -1, "hjtoken", 1234567890123, 1234567890123},
						{"linshu", "lspass", 9, -1, -1, "lstoken", 1234567890123, 1234567890123},
						{"wangshuai", "wspass", 8, 360000, -1, "wstoken", 1234567890123, 1234567890123},
						{"jinyongxue", "jyxpass", 8, 240000, 500, "jyxtoken", 1234567890123, 1234567890123},
						{"liqixing", "lqxpass", 7, 120000, 10, "lqxtoken", 1234567890123, 1234567890123},
						{"huangyelin", "hylpass", 7, 120000, 10, "hyltoken", 1234567890123, 1234567890123}};

void testAdd()
{
	printf("\n=================testAdd start====================\n");
	int ret = 0;
	for(int i = 0; i < 6; i++)
	{
		Poco::Timestamp begin;
		ret = ur->addUser(user[i]);
		Poco::Timestamp end;
		printf("add user %s ret:%d using %llu microseconds\n", user[i].username.c_str(), ret, end - begin);
	}
	printf("==================testAdd finish=================\n");
}

void testGet()
{
	printf("\n=================testGet start====================\n");
	UserRecordNode u1 = {"huangjian", "", 0, 0, 0, "", 0, 0};
	int ret = 0;
	Poco::Timestamp begin1;
	ret = ur->getUserByName(u1);
	Poco::Timestamp end1;
	printf("getUserByName ret %d using %llu microseconds\n"
			"username:%s\n"
			"password:%s\n"
			"authority:%d\n"
			"timeOfValidity:%lld\n"
			"remainOpen:%d\n"
			"token:%s\n"
			"lastVerify:%lld\n"
			"lastLogin:%lld\n",
			ret, end1 - begin1,
			u1.username.c_str(), u1.password.c_str(), u1.authority, u1.timeOfValidity, u1.remainOpen, u1.token.c_str(), u1.lastVerify, u1.lastLogin
			);
	UserRecordNode u2 = {"", "", 0, 0, 0, "lstoken", 0,0 };
	Poco::Timestamp begin2;
	ret = ur->getUserByToken(u2);
	Poco::Timestamp end2;
	printf("\ngetUserByToken ret %d using %llu microseconds\n"
			"username:%s\n"
			"password:%s\n"
			"authority:%d\n"
			"timeOfValidity:%lld\n"
			"remainOpen:%d\n"
			"token:%s\n"
			"lastVerify:%lld\n"
			"lastLogin:%lld\n",
			ret, end2 - begin2,
			u2.username.c_str(), u2.password.c_str(), u2.authority, u2.timeOfValidity, u2.remainOpen, u2.token.c_str(), u2.lastVerify, u2.lastLogin
			);
	
	std::vector<UserRecordNode> vec;
	Poco::Timestamp begin3;
	ret= ur->getUsersByAuth(8, vec);
	Poco::Timestamp end3;
	printf("\ngetUsersByAuth, auth=8, ret %d, vec size %u using %llu microseconds\n", ret, vec.size(), end3 - begin3);
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		UserRecordNode urn = vec[i];
		printf("\ngetUsersByAuth user%u\n"
			"username:%s\n"
			"password:%s\n"
			"authority:%d\n"
			"timeOfValidity:%lld\n"
			"remainOpen:%d\n"
			"token:%s\n"
			"lastVerify:%lld\n"
			"lastLogin:%lld\n",
			i,
			urn.username.c_str(), urn.password.c_str(), urn.authority, urn.timeOfValidity, urn.remainOpen, urn.token.c_str(), urn.lastVerify, urn.lastLogin
			);
	}

	vec.clear();
	Poco::Timestamp begin4;
	ret = ur->getUsersByOpen(10, vec);
	Poco::Timestamp end4;
	printf("getUsersByOpen, open=120000, ret %d, vec size %u using %llu microseconds\n", ret, vec.size(), end4 - begin4);
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		UserRecordNode urn = vec[i];
		printf("\ngetUsersByAuth user%u\n"
			"username:%s\n"
			"password:%s\n"
			"authority:%d\n"
			"timeOfValidity:%lld\n"
			"remainOpen:%d\n"
			"token:%s\n"
			"lastVerify:%lld\n"
			"lastLogin:%lld\n",
			i,
			urn.username.c_str(), urn.password.c_str(), urn.authority, urn.timeOfValidity, urn.remainOpen, urn.token.c_str(), urn.lastVerify, urn.lastLogin
			);
	}
	printf("==================testGet finish=================\n");
}

void testUpdate()
{
	printf("\n=================testUpdate start====================\n");
	UserRecordNode u = {"huangjian", "", 0, 0, 0, "", 0, 0};
	ur->getUserByName(u);
	u.password = "hjnewpass";
	Poco::Timestamp begin;
	ur->updateUser(u);
	Poco::Timestamp end;
	UserRecordNode u2 = {"huangjian", "", 0, 0, 0, "", 0, 0};
	ur->getUserByName(u2);
	printf("after updateUser using %llu to update\n"
			"username:%s\n"
			"password:%s\n"
			"authority:%d\n"
			"timeOfValidity:%lld\n"
			"remainOpen:%d\n"
			"token:%s\n"
			"lastVerify:%lld\n"
			"lastLogin:%lld\n",
			end - begin,
			u2.username.c_str(), u2.password.c_str(), u2.authority, u2.timeOfValidity, u2.remainOpen, u2.token.c_str(), u2.lastVerify, u2.lastLogin
			);

	printf("==================testUpdate finish=================\n");
}

void testDelete()
{
	printf("\n=================testDelete start====================\n");
	for(int i = 0; i < 6; i++)
	{
		Poco::Timestamp begin;
		ur->deleteUser(user[i]);
		Poco::Timestamp end;
		printf("delete %s using %llu microseconds\n", user[i].username.c_str(), end - begin);
	}
	printf("==================testDelete finish=================\n");
}

int main(int argc, char** argv)
{
	ur = CUserRecord::instance();
	ur->init("/home/hj/Dev_Env/Shuchuang/test.db");
	testAdd();
	getchar();
	testGet();
	getchar();
	testUpdate();
	getchar();
	testDelete();
	return 0;
}

