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

void showTasksList()
{
	printf("Totally %d tasks:\n", tasksNum);
	for(int i = 0; i < tasksNum; i++)
	{
		printf("Task %d:\n"
				"{\n"
				"\toption:%d\n"
				"\thour:%d\n"
				"\tminute:%d\n"
				"\tweekday:%x\n"
				"}\n", i + 1, pTask[i]->option, pTask[i]->hour, pTask[i]->minute, pTask[i]->weekday);
	}
}

void displayRecords(const char* buf)
{
	JSON::Parser parser;
	Dynamic::Var var;
	try
	{
		var = parser.parse(buf);
	}
	catch(Exception& e)
	{
		printf("Format error.\n");
		return;
	}
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	JSON::Object::Ptr pParam = pObj->getObject("param");
	JSON::Array::Ptr pArray = pParam->getArray("records");
	unsigned int size = pArray->size();
	for(unsigned int i = 0; i < size; i++)
	{
		JSON::Object::Ptr pRecord = pArray->getObject(i);
		Int64 ms = pRecord->getValue<Int64>("Timestamp");
		int op = pRecord->getValue<int>("Operation");
		int schema = pRecord->getValue<int>("Schema");
		std::string user = pRecord->getValue<std::string>("Username");
		Timestamp ts(ms);
		DateTime dt(ts);
		printf("record %u:\t%04d-%02d-%02d %02d:%02d:%02d\t%s\t%s\t%s\n",
				i, dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(),
				user.c_str(), op==0?"close":"open", schema==0?"manual":"schema");
	}
}

void updateTasksList(const char* buf)
{
	JSON::Parser parser;
	Dynamic::Var var;
	try
	{
		var = parser.parse(buf);
	}
	catch(Exception& e)
	{
		printf("%s\n", e.message().c_str());
		return;
	}
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj.isNull())
	{
		printf("pObj is null.\n");
		return;
	}
	JSON::Object::Ptr pParam = pObj->getObject("param");
	if(pParam.isNull())
	{
		printf("pParam is null.\n");
		return;
	}
	JSON::Array::Ptr pArray = pParam->getArray("tasks");
	tasksNum = pArray->size();
	for(unsigned int i = 0; i < pArray->size(); i++)
	{
		JSON::Object::Ptr pObjTask = pArray->getObject(i);
		if(pObjTask.isNull())
		{
			printf("%u task error.\n", i);
			continue;
		}
		pTask[i]->id = pObjTask->getValue<Int64>("id");
		pTask[i]->option = pObjTask->getValue<int>("option");
		pTask[i]->hour = pObjTask->getValue<int>("hour");
		pTask[i]->minute = pObjTask->getValue<int>("minute");
		pTask[i]->weekday = pObjTask->getValue<int>("weekday");
	}
}
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
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
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
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
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
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
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
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
	updateTasksList(g_buf.c_str());
	showTasksList();
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
	DynamicStruct dsTask;
	printf("\n>>>>    Add task    <<<<\n");
	printf("Task option:");
	int option;
	std::cin >> option;
	printf("Hour:");
	int hour;
	std::cin >> hour;
	printf("Minute:");
	int minute;
	std::cin >> minute;
	printf("Weekday:");
	char str[9];
	std::cin >> str;
	unsigned short mask = 0x40;
	int weekday = 0;
	for(int i = 0; i < 7; i++)
	{
		if(str[i] == '1')
			weekday |= mask;
		mask = mask >> 1;
	}
	printf("%x\n", weekday);
	dsTask["option"] = option;
	dsTask["hour"] = hour;
	dsTask["minute"] = minute;
	dsTask["weekday"] = weekday;
	param["task"] = dsTask;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void deleteTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	printf("Which task to be removed?\n");
	int choice;
	std::cin >> choice;
	Int64 id = pTask[choice - 1]->id;
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.remove";
	DynamicStruct task;
	task["id"] = id;
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	param["task"] = task;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void updateTask()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	printf("Which task to be modified?\n");
	int choice;
	std::cin >> choice;
	printf("Option:");
	int option;
	std::cin >> option;
	int hour;
	printf("Hour:");
	std::cin >> hour;
	printf("Minute:");
	int minute;
	std::cin >> minute;
	printf("Weekday:");
	char str[9];
	std::cin >> str;
	unsigned short mask = 0x40;
	int weekday = 0;
	for(int i = 0; i < 7; i++)
	{
		if(str[i] == '1')
			weekday |= mask;
		mask = mask >> 1;
	}
	printf("%x\n", weekday);
 
	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "task.modify";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	DynamicStruct dsTask;
	dsTask["id"] = pTask[choice-1]->id;
	dsTask["option"] = option;
	dsTask["hour"] = hour;
	dsTask["minute"] = minute;
	dsTask["weekday"] = weekday;
	param["task"] = dsTask;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
}

void getOpRecords()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	std::cout << "Get records>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
	std::cout << "Enter the start time" << std::endl;
	std::cout << "Year:";
	int year_start;
	std::cin >> year_start;
	std::cout << "Month:";
	int month_start;
	std::cin >> month_start;
	std::cout << "Day:";
	int day_start;
	std::cin >> day_start;
	std::cout << "Hour:";
	int hour_start;
	std::cin >> hour_start;
	std::cout << "Minute:";
	int minute_start;
	std::cin >> minute_start;
	DateTime dt_start(year_start, month_start, day_start, hour_start, minute_start);

	std::cout << "Enter the end time" << std::endl;
	std::cout << "Year:";
	int year_end;
	std::cin >> year_end;
	std::cout << "Month:";
	int month_end;
	std::cin >> month_end;
	std::cout << "Day:";
	int day_end;
	std::cin >> day_end;
	std::cout << "Hour:";
	int hour_end;
	std::cin >> hour_end;
	std::cout << "Minute:";
	int minute_end;
	std::cin >> minute_end;
	DateTime dt_end(year_end, month_end, day_end, hour_end, minute_end);

	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "record.get";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	Int64 ts_start = dt_start.timestamp().epochMicroseconds();
	Int64 ts_end = dt_end.timestamp().epochMicroseconds();
	param["starttime"] = ts_start;
	param["endtime"] = ts_end;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
	displayRecords(g_buf.c_str());
}

void deleteOpRecord()
{
	if(g_token.empty())
	{
		printf("Please login first.\n");
		return;
	}
	std::cout << "Delete records>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
	std::cout << "Enter the start time" << std::endl;
	std::cout << "Year:";
	int year_start;
	std::cin >> year_start;
	std::cout << "Month:";
	int month_start;
	std::cin >> month_start;
	std::cout << "Day:";
	int day_start;
	std::cin >> day_start;
	std::cout << "Hour:";
	int hour_start;
	std::cin >> hour_start;
	std::cout << "Minute:";
	int minute_start;
	std::cin >> minute_start;
	DateTime dt_start(year_start, month_start, day_start, hour_start, minute_start);

	std::cout << "Enter the end time" << std::endl;
	std::cout << "Year:";
	int year_end;
	std::cin >> year_end;
	std::cout << "Month:";
	int month_end;
	std::cin >> month_end;
	std::cout << "Day:";
	int day_end;
	std::cin >> day_end;
	std::cout << "Hour:";
	int hour_end;
	std::cin >> hour_end;
	std::cout << "Minute:";
	int minute_end;
	std::cin >> minute_end;
	DateTime dt_end(year_end, month_end, day_end, hour_end, minute_end);

	DynamicStruct ds;
	ds["type"] = "request";
	ds["action"] = "record.delete";
	DynamicStruct param;
	param["uuid"] = g_uuid;
	param["token"] = g_token;
	Int64 ts_start = dt_start.timestamp().epochMicroseconds();
	Int64 ts_end = dt_end.timestamp().epochMicroseconds();
	param["starttime"] = ts_start;
	param["endtime"] = ts_end;
	ds["param"] = param;
	if(!sendRequest(ds.toString()))
	{
		warnf("%s, %d: Test faild\n", __FILE__, __LINE__);
		return;
	}
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

void showRecordOperation()
{
	while(1)
	{
		std::cout << "Action:" << std::endl << "1.Get operation records" << std::endl;
		std::cout << "0.Return" << std::endl;
		int choice;
		std::cin >> choice;
		switch(choice)
		{
			case 1:
				getOpRecords();
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
	g_pContext = new Context(Context::TLSV1_2_CLIENT_USE, "", Context::VERIFY_NONE);
	pTask = new TaskInfo* [20];
	for(int i = 0; i < 20; i++)
	{
		pTask[i] = new TaskInfo;
	}
	while(1)
	{
		std::cout << "1.User operation" << std::endl << "2.Door operation" << std::endl << "3.Task operation" << std::endl;
		std::cout << "4.Record operation" << std::endl;
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

