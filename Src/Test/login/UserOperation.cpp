#include <iostream>
#include "Poco/Dynamic/Struct.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Common/PrintLog.h"
#include "Poco/SHA1Engine.h"
#include "Poco/DateTime.h"

using namespace Poco;
extern std::string g_uuid;
extern std::string g_username;
extern std::string g_token;
extern std::string g_buf; 
extern bool sendRequest(std::string content);
extern std::string generateMD5Password(std::string prefix, std::string password, std::string challenge);
void login();
void logout();
void passwd();
void addUser();
void deleteUser();
void topUpUser();
void listUser();
void showUserOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Login" << std::endl << "2.Change password" << std::endl;
		std::cout << "3.Logout" << std::endl << "4.Add user" << std::endl << "5.Delete user"<< std::endl << "6.Top up user" << std::endl << "7.List user" << std::endl  << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				login();
				break;
			case 2:
				passwd();
				break;
			case 3:
				logout();
				break;
			case 4:
				addUser();
				break;
			case 5:
				deleteUser();
				break;
			case 6:
				topUpUser();
				break;
			case 7:
				listUser();
				break;
			case 0:
				return;
			default:
				break;
		}
	}
}

void login()
{
	std::cout << "Uuid:";
	std::cin >> g_uuid;
	std::cout << "Username:";
	std::cin >> g_username;
	g_token = "";
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.login";
	DynamicStruct param;
	param["username"] = g_username;
	param["uuid"] = g_uuid;
	std::string binduser;
	std::cout << "Binduser:";
	std::cin >> binduser;
	param["binduser"] = binduser;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

	JSON::Parser parser;
	Dynamic::Var var = parser.parse(g_buf);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	DynamicStruct dss = *pObj;
	if(dss["result"].toString() != "good")
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
	std::string param_str = dss["param"].toString();
	parser.reset();
	Dynamic::Var varr = parser.parse(param_str.c_str());
	JSON::Object::Ptr pObjj = varr.extract<JSON::Object::Ptr>();
	DynamicStruct dparam = *pObjj;
	std::string challenge = dparam["challenge"];
	std::string token = dparam["token"];
	tracef("%s, %d: token %s, challenge %s\n", __FILE__, __LINE__, token.c_str(), challenge.c_str());
	g_token = token;
	std::cout << "Login step 1 finished, go on step 2?"<< std::endl <<	"y/n	";
	char choice;
	std::cin >> choice;
	if(choice != 'y')
		return;

	std::cout << "Password:";
	std::string pass;
	std::cin >> pass;
	std::string passmd5 = generateMD5Password("login", pass, challenge);;
	param["password"] = passmd5;
	param["token"] = token;
	param["mobiletoken"] = "ec77339fd0393bd84155b383501e368eb116801532ffcba775bf0d208f384d0a"; 
	ds["param"] = param;

	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void passwd()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.passwd";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}

	JSON::Parser parser;
	Dynamic::Var var = parser.parse(g_buf);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj.isNull() || !pObj->has("result"))
	{
		printf("error in passwd.\n");
		return;
	}
	if(pObj->getValue<std::string>("result") != "good")
	{
		std::string detail = "";
		if(pObj->has("detail"))
			detail = pObj->getValue<std::string>("detail");
		printf("passwd failed, reason:%s", detail.c_str());
		return;
	}
	JSON::Object::Ptr pParam = pObj->getObject("param");
	std::string challenge = pParam->getValue<std::string>("challenge");
	std::string oldpass;
	std::cout << "Old password:" << std::endl;
	std::cin >> oldpass;
	std::string challengemd5pass = generateMD5Password("passwd", oldpass, challenge);
	std::string pass;
	std::string pass2;
	do
	{
		std::cout << "New password:" << std::endl;
		std::cin >> pass;
		std::cout << "Confirm new password:" << std::endl;
		std::cin >> pass2;
	}while(pass != pass2);
	SHA1Engine sha1;
	sha1.update(pass);
	const DigestEngine::Digest& digestNewPass = sha1.digest();
	std::string md5newPassword(DigestEngine::digestToHex(digestNewPass));
	DynamicStruct ds2;
	ds2["type"] = "request";
	ds2["action"] = "user.passwd";
	DynamicStruct param2;
	param2["uuid"] = g_uuid;
	param2["token"] = g_token;
	param2["password"] = challengemd5pass;
	param2["newpassword"] = md5newPassword;
	ds2["param"] = param2;
	if(!sendRequest(ds2.toString()))
	{

		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void logout()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.logout";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void addUser()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.add";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	std::string username;
	std::cout << "Username:";
	std::cin >> username;
	param["username"] = username;
	std::cout << "Binduser:";
	std::string binduser;
	std::cin >> binduser;
	param["binduser"] = binduser;
	std::string password;
	std::cout << "Password:";
	std::cin >> password;
	SHA1Engine sha1;
	sha1.update(password);
	const DigestEngine::Digest& digestPass = sha1.digest();
	std::string sha1pass = DigestEngine::digestToHex(digestPass);
	param["password"] = sha1pass;
	std::cout << "Authority:";
	int auth;
	std::cin >> auth;
	param["authority"] = auth;
	std::cout << "RemainOpen:";
	int remainOpen = 0;
	std::cin >> remainOpen;
	param["remainopen"] = remainOpen;
	std::cout << "TimeOfValidity:" << std::endl << "Year:";
	int year;
	std::cin >> year;
	std::cout << "Month:";
	int month;
	std::cin >> month;
	std::cout << "Day:";
	int day;
	std::cin >> day;
	DateTime date(year, month, day);
	param["timeofvalidity"] = date.timestamp().epochMicroseconds();
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test failed\n", __FILE__, __LINE__);
		return;
	}
}

void deleteUser()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.delete";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	std::string username;
	std::cout << "Username:";
	std::cin >> username;
	param["username"] = username;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test failed\n", __FILE__, __LINE__);
		return;
	}
}

void topUpUser()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.topup";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	std::string username;
	std::cout << "Username:";
	std::cin >> username;
	int remainOpen;
	std::cout << "RemainOpen:";
	std::cin >> remainOpen;
	param["username"] = username;
	param["remainopen"] = remainOpen;
	std::cout << "TimeOfValidity:" << std::endl << "Year:";
	int year;
	std::cin >> year;
	std::cout << "Month:";
	int month;
	std::cin >> month;
	std::cout << "Day:";
	int day;
	std::cin >> day;
	DateTime date(year, month, day);
	param["timeofvalidity"] = date.timestamp().epochMicroseconds();
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test failed\n", __FILE__, __LINE__);
		return;
	}
}

void listUser()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.list";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	param["limit"] = 10;
	param["offset"] = 0;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test failed\n", __FILE__, __LINE__);
		return;
	}

}

