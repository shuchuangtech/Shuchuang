#include <stdio.h>
#include "Device/Component/User/UserManager.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/MD5Engine.h"
using namespace Poco;
CUserManager* user_manager;
std::string token = "";
std::string generateMD5Password(const std::string& prefix, const std::string& challenge, const std::string& password)
{
	MD5Engine md5;
	md5.update(password);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5_local_password(DigestEngine::digestToHex(digest));
	std::string prefix_password = prefix + md5_local_password;
	md5.reset();
	md5.update(prefix_password);
	const DigestEngine::Digest& digest2 = md5.digest();
	std::string md5_prefix_password(DigestEngine::digestToHex(digest2));
	std::string challenge_password = md5_prefix_password + challenge;
	md5.reset();
	md5.update(challenge_password);
	const DigestEngine::Digest& digest3 = md5.digest();
	std::string md5_challenge_password(DigestEngine::digestToHex(digest3));
	return md5_challenge_password;
}

void testLogin()
{
	printf("test login\nusername:");
	char un[16];
	scanf("%s", un);
	printf("password:");
	char pw[16];
	scanf("%s", pw);
	JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("username", std::string(un));
	std::string detail;
	user_manager->login(pObj, detail);
	Poco::DynamicStruct ds = *pObj;
	printf("login step 1:param:%s, detail:%s\n", ds.toString().c_str(), detail.c_str());
	std::string challenge = pObj->getValue<std::string>("challenge");
	token = pObj->getValue<std::string>("token");
	std::string md5password = generateMD5Password("login", challenge, pw);

	pObj = NULL;
	pObj = new JSON::Object;
	pObj->set("username", "huangjian");
	pObj->set("token", token);
	pObj->set("password", md5password);
	bool ret = user_manager->login(pObj, detail);
	if(ret)
	{
		printf("login successfully.\n");
	}
	else
	{
		printf("login failed,%s.\n", detail.c_str());
	}
}

void testPasswd()
{
	printf("test login\n");
	if(token.empty())
	{
		printf("token is empty, please login first.");
	}
	JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("token", token);
	std::string detail = "";
	user_manager->passwd(pObj, detail);
	DynamicStruct ds = *pObj;
	printf("passwd param:%s, detail:%s\n", ds.toString().c_str(), detail.c_str());
	pObj = NULL;
	std::string challenge = ds["challenge"].toString();
	char pw[16];
	printf("old password:");
	scanf("%s", pw);
	char newpw[16];
	printf("new password:");
	scanf("%s", newpw);
	std::string md5password = generateMD5Password("passwd", challenge, pw);
	MD5Engine md5;
	md5.update(newpw);
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5newpasswd(DigestEngine::digestToHex(digest));
	pObj = new JSON::Object;
	pObj->set("token", token);
	pObj->set("password", md5password);
	pObj->set("newpassword", md5newpasswd);
	if(user_manager->passwd(pObj, detail))
	{
		printf("change password successfully.\n");
	}
	else
	{
		printf("change password failed, %s\n", detail.c_str());
	}
}

void testLogout()
{
	printf("test logout\n");
	if(token.empty())
	{
		printf("token is empty, please login first.");
	}
	JSON::Object::Ptr pObj = new JSON::Object;
	pObj->set("token", token);
	std::string detail = "";
	if(user_manager->logout(pObj, detail))
	{
		printf("logout successfully.\n");
	}
	else
	{
		printf("logout failed, %s.\n", detail.c_str());
	}

}

int main(int argc, char** argv)
{
	user_manager = CUserManager::instance();
	user_manager->init("/home/hj/Dev_Env/Shuchuang/test.db");
	testLogin();
	testPasswd();
	testLogout();
	//testCreate();
	//testDelete();
	return 0;	
}

