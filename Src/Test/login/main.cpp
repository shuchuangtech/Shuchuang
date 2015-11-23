#include "Poco/Types.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Dynamic/Struct.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/MediaType.h"
#include "Common/PrintLog.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/MD5Engine.h"
#include "Device/Component/Task/TaskInfo.h"
using namespace Poco;
using namespace Poco::Net;
char g_buf[1024];
Context::Ptr  g_pContext = NULL;
std::string g_token = "";
std::string g_host;
int g_port;
std::string g_uuid;
std::string g_username;

std::string generateMD5Password(std::string prefix, std::string password, std::string challenge)
{
	MD5Engine md5;
	md5.update(password);
	const DigestEngine::Digest& digestPrefix = md5.digest();
	std::string md5pass(DigestEngine::digestToHex(digestPrefix));

	std::string prefix_passwd = prefix + md5pass;
	md5.reset();
	md5.update(prefix_passwd);
	const DigestEngine::Digest& digest = md5.digest();
	std::string prefixpassmd5(DigestEngine::digestToHex(digest));
	prefixpassmd5 += challenge;

	md5.reset();
	md5.update(prefixpassmd5);
	const DigestEngine::Digest& dg = md5.digest();
	std::string passs(DigestEngine::digestToHex(dg));
	return passs;
}

void sendRequest(std::string content, char* buf)
{
	tracef("%s, %d: SendRequest:%s", __FILE__, __LINE__, content.c_str());
	HTTPSClientSession https(g_host, (UInt16)g_port, g_pContext);
	HTTPRequest request;
	request.setContentType(MediaType("application", "json"));
	request.setKeepAlive(true);
	request.setContentLength(content.length());
	std::ostream& ostr = https.sendRequest(request);
	ostr << content << std::flush;
	HTTPResponse response;
	std::istream& istr = https.receiveResponse(response);
	istr.read(buf, 1024);
	tracef("%s, %d: Receive buf: %s\n", __FILE__, __LINE__, buf);
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
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);

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
	ds["param"] = param;

	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
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
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);

	JSON::Parser parser;
	Dynamic::Var var = parser.parse(g_buf);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
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
	MD5Engine md5;
	md5.update(pass);
	const DigestEngine::Digest& digestNewPass = md5.digest();
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
	memset(g_buf, 0, 1024);
	sendRequest(ds2.toString(), g_buf);
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
	param["token"] = g_username;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void showUserOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Login" << std::endl << "2.Change password" << std::endl;
		std::cout << "3.Logout" << std::endl << "0.Return" << std::endl;
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
			case 0:
				return;
			default:
				break;
		}
	}
}

void checkDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.check";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void openDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.open";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void closeDoor()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "device.close";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void getTasks()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.list";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void addTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.add";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	DynamicStruct dsTask;
	ds["task"] = dsTask;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void deleteTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.remove";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void updateTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.modify";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void showDoorOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Check" << std::endl << "2.Open" << std::endl;
		std::cout << "3.Close" << std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				checkDoor();
				break;
			case 2:
				openDoor();
				break;
			case 3:
				closeDoor();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}

void showTaskOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Get tasks" << std::endl << "2.Add task" << std::endl;
		std::cout << "3.Delete task" << std::endl << "4.Update task"<< std::endl << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				getTasks();
				break;
			case 2:
				addTask();
				break;
			case 3:
				deleteTask();
				break;
			case 4:
				updateTask();
				break;
			case 0:
				return;
			default:
				break;
		}
	}

}

int main(int argc, char** argv)
{
	std::cout << "Host:";
	std::string host;
	std::cin >> host;
	g_host = host;
	std::cout << "Port:";
	int port;
	std::cin >> port;
	g_port = port;
	g_pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
	while(1)
	{
		std::cout << "1.User operation" << std::endl << "2.Door operation" << std::endl << "3.Task operation" << std::endl;
		std::cout << "0.Exit" << std::endl;
		int choice;
		std::cin >> choice;
		if(choice == 0)
		{
			break;
		}
		switch(choice)
		{
			case 1:
				showUserOperation();break;
			case 2:
				showDoorOperation();break;
			case 3:
				showTaskOperation();break;
			default:
				std::cout << "Error choice" << std::endl;
				break;
		}
	}
	return 0;
}

