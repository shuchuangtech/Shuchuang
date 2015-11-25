#ifndef __DEVICE_COMPONENT_RECORD_OP_MANAGER_H__
#define __DEVICE_COMPONENT_RECORD_OP_MANAGER_H__
#include "Poco/Runnable.h"
#include "Poco/SingletonHolder.h"
#include "Device/Util/OperationRecord.h"
#include "Poco/Thread.h"
#include "Poco/JSON/Object.h"
class COpManager : public Poco::Runnable
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
	void run();
	bool getRecords(Poco::JSON::Object::Ptr& param, std::string& detail);
//	bool deleteReocrds(Poco::JSON::Object::Ptr& param, std::string& detail);
private:
	COperationRecord*	m_op_record;
	bool				m_started;
	Poco::Thread*		m_thread;
};
#endif

