#include "Device/Component/Record/OperationManager.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/DateTime.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/JSON/Array.h"
using namespace Poco;
COpManager::COpManager()
{
	m_started = false;
	m_op_record = NULL;
	m_thread = NULL;
}

COpManager::~COpManager()
{
}

bool COpManager::init(const std::string& dbPath)
{
	m_op_record = COperationRecord::instance();
	return m_op_record->init(dbPath);
}

bool COpManager::start()
{
	if(m_started)
	{
		warnf("%s, %d: OpManager already started.", __FILE__, __LINE__);
		return false;
	}
	m_thread = new Thread("OpManager");
	m_started = true;
	m_thread->start(*this);
	return true;
}

bool COpManager::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: OpManager not running.", __FILE__, __LINE__);
		return false;
	}
	m_thread->join();
	m_started = false;
	return true;
}

void COpManager::run()
{
	while(m_started)
	{
		DateTime now;
		//sleep 1 seconds, 
		Thread::sleep(1000);
		DateTime today(now.year(),
						now.month(),
						now.day(),
						0,
						0,
						0);
		Timespan oneday(1, 0, 0, 0, 0);
		Timespan tenday(10, 0, 0, 0, 0);
		Timespan nextRun = today + oneday - now;
		int sleepVal = nextRun.totalSeconds();
		DateTime deleteDay = today - tenday;
		int ret = m_op_record->deleteRecordsByDate(deleteDay);
		infof("%s, %d: Operation records[num %d] on %04d-%02d-%02d deleted.", __FILE__, __LINE__, ret, deleteDay.year(), deleteDay.month(), deleteDay.day());
		Thread::sleep(sleepVal * 1000);
	}
}

bool COpManager::getRecords(JSON::Object::Ptr& param, std::string& detail)
{
	if(param.isNull() || !param->has(RECORD_STARTTIME_STR) || !param->has(RECORD_ENDTIME_STR))
	{
		detail = "440";
		return false;
	}
	Int64 start = param->getValue<Int64>(RECORD_STARTTIME_STR);
	Int64 end = param->getValue<Int64>(RECORD_ENDTIME_STR);
	Timestamp tsstart(start);
	Timestamp tsend(end);
	std::vector<OperationRecordNode> data_set;
	data_set.clear();
	int ret = m_op_record->getRecords(tsstart, tsend, data_set);
	JSON::Array::Ptr pArray = new JSON::Array;
	if(ret > 0)
	{
		for(int i = 0; i < ret; i++)
		{
			DynamicStruct ds;
			ds["Timestamp"] = data_set[i].timestamp;
			ds["Operation"] = data_set[i].operation;
			ds["Username"] = data_set[i].username;
			ds["Schema"] = data_set[i].schema;
			pArray->add(ds);
		}
	}
	param->set(RECORD_RECORDS_STR, pArray);
	infof("%s, %d: %d operation records return.", __FILE__, __LINE__, ret);
	return true;
}
/*
bool deleteRecords(JSON::Object::Ptr& param, std::string& detail)
{
	if(param.isNull() || !param->has(RECORD_STARTTIME_STR) || !param->has(RECORD_ENDTIME_STR))
	{
		detail = "440";
		return false;
	}
	Int64 start = param->getValue<Int64>(RECORD_STARTTIME_STR);
	Int64 end = param->getValue<Int64>(RECORD_ENDTIME_STR);
	Timestamp tsstart(start);
	Timestamp tsend(end);
	int ret = m_op_record->deleteRecords(tsstart, tsend);
	infof("%s, %d: %d operation records from %lld to %lld deleted.", __FILE__, __LINE__, ret, start, end);
	return true;
}
*/
