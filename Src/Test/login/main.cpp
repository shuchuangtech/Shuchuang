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
using namespace Poco;
using namespace Poco::Net;
char g_buf[1024];
Context::Ptr  g_pContext = NULL;
std::string g_token = "";
std::string g_host;
int g_port;
std::string g_uuid;
std::string g_username;
void sendRequest(std::string content, char* buf)
{
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
	tracef("%s, %d: buf: %s\n", __FILE__, __LINE__, buf);
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
	MD5Engine md5;
	md5.update(pass);
	const DigestEngine::Digest& digest = md5.digest();
	std::string passmd5(DigestEngine::digestToHex(digest));
	std::string login = "login" + passmd5;

	tracef("%s, %d: password %s\n", __FILE__, __LINE__, passmd5.c_str());
	md5.reset();
	md5.update(login);
	const DigestEngine::Digest& digest2 = md5.digest();
	std::string loginpassmd5(DigestEngine::digestToHex(digest));
	loginpassmd5 += challenge;

	md5.reset();
	md5.update(loginpassmd5);
	const DigestEngine::Digest& dg = md5.digest();
	std::string passs(DigestEngine::digestToHex(digest));
	tracef("%s, %d: password %s\n", __FILE__, __LINE__, passs.c_str());
	param["password"] = passs;
	param["token"] = token;
	ds["param"] = param;

	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void passwd()
{
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "user.passwd";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["username"] = g_username;
	param["token"] = g_token;
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
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5password(DigestEngine::digestToHex(digest));
	param["password"] = md5password;
	ds["param"] = param;
	memset(g_buf, 0, 1024);
	sendRequest(ds.toString(), g_buf);
}

void logout()
{
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
		std::cout << "Action:" << std::endl << "1.Login" << std::endl << "2.Change password" << std::endl;
		std::cout << "3.Logout" << std::endl << "0.Exit" << std::endl;
		int choice;
		std::cin >> choice;
		if(choice == 0)
		{
			break;
		}
		switch(choice)
		{
			case 1:
				login();break;
			case 2:
				passwd();break;
			case 3:
				logout();break;
			default:
				std::cout << "Error choice" << std::endl;
				break;
		}
	}
	return 0;
}

