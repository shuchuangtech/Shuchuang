#ifndef __DEVICE_COMPONENT_RECORD_OP_MANAGER_H__
#define __DEVICE_COMPONENT_RECORD_OP_MANAGER_H__
#include "Poco/SingletonHolder.h"
#include "Device/Util/OperationRecord.h"
#include "Poco/Timer.h"
#include "Poco/DateTime.h"
#include "Poco/JSON/Object.h"
#include "Poco/Mutex.h"
#include "Device/Notification/MessageNotification.h"
class COpManager
{
public:
	static COpManager* instance()
	{
		static Poco::SingletonHolder<COpManager> sh;
		return sh.get();
	}
	COpManager();
	~COpManager();
	bool init(const std::string& dbPath);
	bool start();
	bool stop();
	void timerCallback(Poco::Timer& timer);
	//interface to RPCClient
	bool getRecords(Poco::JSON::Object::Ptr& param, std::string& detail);
	//interface to DeviceControler
	bool addRecord(OperationRecordNode& node);
	void handleNotification(MessageNotification* pNf);
private:
	void writeAllRecords();
	COperationRecord*	m_op_record;
	bool				m_started;
	Poco::Timer*		m_timer;
	Poco::Mutex			m_mutex;
	std::vector<OperationRecordNode>		m_cache_map;
	Poco::DateTime		m_lastDeleteTime;
};
#endif

