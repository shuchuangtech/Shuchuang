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
	m_cache_map.clear();
	m_thread = new Thread("OpManager");
	m_started = true;
	m_thread->start(*this);
	infof("%s, %d: Operation record manager start successfully.", __FILE__, __LINE__);
	return true;
}

bool COpManager::stop()
{
	if(!m_started)
	{
		warnf("%s, %d: OpManager not running.", __FILE__, __LINE__);
		return false;
	}
	writeAllRecords();
	m_thread->join();
	m_started = false;
	infof("%s, %d: Operation record manager stop successfully.", __FILE__, __LINE__);
	return true;
}

void COpManager::writeAllRecords()
{
	Mutex::ScopedLock lock(m_mutex);
	if(m_cache_map.size() > 0)
	{
		int ret = m_op_record->addRecord(m_cache_map);
		if(ret != m_cache_map.size())
			warnf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
		m_cache_map.clear();
		infof("%s, %d: %d records insert into database.", __FILE__, __LINE__, ret);
	}
}

void COpManager::run()
{
	//set lastCheckTime to 2000-1-1 0:0:0 to triger process
	DateTime lastDeleteTime(2000, 1, 1, 0, 0, 0);
	while(m_started)
	{
		writeAllRecords();
		DateTime now;
		Timespan diff = now - lastDeleteTime;
		if(diff.days() > 0)
		{
			DateTime today(now.year(),
						now.month(),
						now.day(),
						0,
						0,
						0);
			Timespan tenday(10, 0, 0, 0, 0);
			DateTime deleteDay = today - tenday;
			int ret = m_op_record->deleteRecordsByDate(deleteDay);
			infof("%s, %d: Operation %d records on %04d-%02d-%02d deleted.", __FILE__, __LINE__, ret, deleteDay.year(), deleteDay.month(), deleteDay.day());
			lastDeleteTime = now;
		}
		Thread::sleep(60 * 1000);
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
	Int64 numStart = tsstart.epochMicroseconds();
	Int64 numEnd = tsend.epochMicroseconds();
	m_mutex.lock();
	if((m_cache_map.size() > 0) && ((m_cache_map.begin())->timestamp <= numEnd)
			&& ((m_cache_map.end())->timestamp >= numStart))
	{
		//find begin and end
		std::vector<OperationRecordNode>::iterator it_begin = m_cache_map.begin();
		std::vector<OperationRecordNode>::iterator it_end = m_cache_map.end();
		bool reverse = false;
		// if it->first begin < numStart, then reverse
		if(it_begin->timestamp < numStart)
		{
			it_begin = m_cache_map.end();
			it_end = m_cache_map.begin();
			reverse = true;
		}
		while(true)
		{
			if(it_begin->timestamp >= numStart && it_begin->timestamp <= numEnd)
			{
				ret++;
				DynamicStruct ds;
				ds["Timestamp"] = it_begin->timestamp;
				ds["Operation"] = it_begin->operation;
				ds["Username"] = it_begin->username;
				ds["Schema"] = it_begin->schema;
				pArray->add(ds);
			}
			else
			{
				break;
			}
			if(it_begin == it_end)
				break;
			if(reverse)
				it_begin--;
			else
				it_begin++;
		}
	}
	m_mutex.unlock();
	DateTime dtstart(tsstart);
	DateTime dtend(tsend);
	param->set(RECORD_RECORDS_STR, pArray);
	infof("%s, %d: %d operation records from %04d-%02d-%02d %02d:%02d:%02d to %04d-%02d-%02d %02d:%02d:%02d return.", __FILE__, __LINE__, ret, 
			dtstart.year(), dtstart.month(), dtstart.day(), dtstart.hour(), dtstart.minute(), dtstart.second(),
			dtend.year(), dtend.month(), dtend.day(), dtend.hour(), dtend.minute(), dtend.second());
	return true;
}

bool COpManager::addRecord(OperationRecordNode& node)
{
	m_mutex.lock();
	m_cache_map.push_back(node);
	m_mutex.unlock();
	return true;
}

