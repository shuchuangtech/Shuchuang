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
#include "Poco/SHA1Engine.h"
#include "Device/Component/Task/TaskInfo.h"
extern void showUserOperation();
extern void showTaskOperation();
extern void showRecordOperation();
extern void showDoorOperation();
extern void showSystemOperation();
extern void showServerOperation();
using namespace Poco;
using namespace Poco::Net;
std::string g_buf = "";
Context::Ptr  g_pContext = NULL;
std::string g_token = "";
std::string g_host;
int g_port;
std::string g_uuid;
std::string g_username;

TaskInfo** pTask = NULL;
int tasksNum;

std::string generateMD5Password(std::string prefix, std::string password, std::string challenge)
{
	MD5Engine md5;
	SHA1Engine sha1;
	sha1.update(password);
	const DigestEngine::Digest& digestPassword = sha1.digest();
	std::string sha1pass(DigestEngine::digestToHex(digestPassword));

	std::string prefix_passwd = prefix + sha1pass;
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

bool sendRequest(std::string content)
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
	https.setTimeout(Timespan(30, 0));
	try
	{
		std::istream& istr = https.receiveResponse(response);
		g_buf = "";
		char buf[1024] = {0, };
		while(!istr.eof())
		{
			istr.read(buf, 1024);
			g_buf += buf;
			memset(buf, 0, 1024);
		}
	}
	catch(Exception& e)
	{
		warnf("%s, %d: %s", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	tracef("%s, %d: Receive buf length:%d, content:\n%s", __FILE__, __LINE__, g_buf.length(), g_buf.c_str());
	return true;
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
	g_pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE, 9, false, "EECDH+aRSA+AESGCM");
	pTask = new TaskInfo* [20];
	for(int i = 0; i < 20; i++)
	{
		pTask[i] = new TaskInfo;
	}
	while(1)
	{
		std::cout << "1.User operation" << std::endl << "2.Door operation" << std::endl << "3.Task operation" << std::endl;
		std::cout << "4.Record operation" << std::endl << "5.System operation" << std::endl << "6.Server operation" << std::endl;
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
			case 4:
				showRecordOperation();break;
			case 5:
				showSystemOperation();break;
			case 6:
				showServerOperation();break;
			default:
				std::cout << "Error choice" << std::endl;
				break;
		}
	}
	for(int i = 0; i < 20; i++)
	{
		delete pTask[i];
	}
	delete[] pTask;
	return 0;
}

