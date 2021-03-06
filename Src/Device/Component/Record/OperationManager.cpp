#include "Device/Component/Record/OperationManager.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/JSON/Array.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
using namespace Poco;
COpManager::COpManager()
:m_lastDeleteTime(2000, 1, 1, 0, 0, 0)
{
	m_started = false;
	m_op_record = NULL;
}

COpManager::~COpManager()
{
}

bool COpManager::init(const std::string& dbPath)
{
	m_op_record = COperationRecord::instance();
	Observer<COpManager, MessageNotification> ob(*this, &COpManager::handleNotification);
	NotificationCenter::defaultCenter().addObserver(ob);
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
	m_timer = new Timer(0, 60 * 1000);
	m_started = true;
	TimerCallback<COpManager> callBack(*this, &COpManager::timerCallback);
	m_timer->start(callBack);
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
	m_timer->stop();
	m_started = false;
	infof("%s, %d: Operation record manager stop successfully.", __FILE__, __LINE__);
	return true;
}

void COpManager::writeAllRecords()
{
	Mutex::ScopedLock lock(m_mutex);
	if(m_cache_map.size() > 0)
	{
		unsigned int ret = m_op_record->addRecord(m_cache_map);
		if(ret != m_cache_map.size())
			warnf("%s, %d: Not supposed to be here.", __FILE__, __LINE__);
		m_cache_map.clear();
		infof("%s, %d: %d records insert into database.", __FILE__, __LINE__, ret);
	}
}

void COpManager::timerCallback(Timer& timer)
{
	//set lastCheckTime to 2000-1-1 0:0:0 to triger process
	writeAllRecords();
	DateTime now;
	Timespan diff = now - m_lastDeleteTime;
	if(diff.days() > 0)
	{
		DateTime today(now.year(),
					now.month(),
					now.day(),
					0,
					0,
					0);
		Timespan thirtyday(30, 0, 0, 0, 0);
		DateTime deleteDay = today - thirtyday;
		int ret = m_op_record->deleteRecordsByDate(deleteDay);
		infof("%s, %d: Operation %d records on %04d-%02d-%02d deleted.", __FILE__, __LINE__, ret, deleteDay.year(), deleteDay.month(), deleteDay.day());
		m_lastDeleteTime = now;
	}
}

void COpManager::handleNotification(MessageNotification* pNf)
{
	MessageNotification::Ptr pNoti(pNf);
	std::string notiName = pNoti->getName();
	if(notiName == "SystemWillReboot")
	{
		infof("%s, %d: OpManager receive SystemWillReboot notification, now write all op records", __FILE__, __LINE__);
		writeAllRecords();
	}
}

bool COpManager::getRecords(JSON::Object::Ptr& param, std::string& detail)
{
	if(param.isNull() || !param->has(RECORD_STARTTIME_STR) || !param->has(RECORD_ENDTIME_STR) || !param->has(RECORD_LIMIT_STR) || !param->has(RECORD_OFFSET_STR))
	{
		detail = "440";
		return false;
	}
	writeAllRecords();
	int limit = param->getValue<int>(RECORD_LIMIT_STR);
	int offset = param->getValue<int>(RECORD_OFFSET_STR);
	Int64 start = param->getValue<Int64>(RECORD_STARTTIME_STR);
	Int64 end = param->getValue<Int64>(RECORD_ENDTIME_STR);
	Timestamp tsstart(start);
	Timestamp tsend(end);
	std::vector<OperationRecordNode> data_set;
	data_set.clear();
	int ret = m_op_record->getRecords(tsstart, tsend, limit, offset, data_set);
	if(ret < 0)
	{
		detail = "441";
		return false;
	}
	JSON::Array::Ptr pArray = new JSON::Array;
	for(int i = 0; i < ret; i++)
	{
		DynamicStruct ds;
		ds["Timestamp"] = data_set[i].timestamp;
		ds["Operation"] = data_set[i].operation;
		ds["Username"] = data_set[i].username;
		ds["Schema"] = data_set[i].schema;
		pArray->add(ds);
	}
	DateTime dtstart(tsstart);
	DateTime dtend(tsend);
	param->set(RECORD_RECORDS_STR, pArray);
	infof("%s, %d: %d operation records from %04d-%02d-%02d %02d:%02d:%02d to %04d-%02d-%02d %02d:%02d:%02d return.", __FILE__, __LINE__, ret, 
			dtstart.year(), dtstart.month(), dtstart.day(), dtstart.hour(), dtstart.minute(), dtstart.second(),
			dtend.year(), dtend.month(), dtend.day(), dtend.hour(), dtend.minute(), dtend.second());
	if(param->has(REG_TOKEN_STR))
	{
		param->remove(REG_TOKEN_STR);
	}
	return true;
}

bool COpManager::addRecord(OperationRecordNode& node)
{
	m_mutex.lock();
	m_cache_map.push_back(node);
	m_mutex.unlock();
	return true;
}

