#include <iostream>
#include "Common/PrintLog.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
using namespace Poco;
extern bool sendRequest(std::string content);
extern std::string g_token;
extern std::string g_uuid;
extern std::string g_buf;
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
	std::cout << "offset";
	int offset;
	std::cin >> offset;
	std::cout << "limit";
	int limit;
	std::cin >> limit;
	param["offset"] = offset;
	param["limit"] = limit;
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

