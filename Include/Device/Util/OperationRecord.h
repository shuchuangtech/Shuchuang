#ifndef __DEVICE_UTIL_OPERATION_RECORD_H__
#define __DEVICE_UTIL_OPERATION_RECORD_H__
#include "Poco/SingletonHolder.h"
#include "Poco/Types.h"
#include "Poco/Data/Session.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include <vector>
struct _OperationRecordNode
{
	Poco::Int64		timestamp;
	int				operation;
	std::string		username;
	int				schema;
};
typedef struct _OperationRecordNode OperationRecordNode;
class COperationRecord
{
public:
	COperationRecord();
	~COperationRecord();
	static COperationRecord* instance()
	{
		static Poco::SingletonHolder<COperationRecord> sh;
		return sh.get();
	}
	bool	init(const std::string& dbPath);
	bool	resetOpRecord();
	int		addRecord(OperationRecordNode& record);
	int		addRecord(std::vector<OperationRecordNode>&);
	int		deleteRecordsByDate(Poco::DateTime& date);
	int		getRecords(Poco::Timestamp& start, Poco::Timestamp& end, std::vector<OperationRecordNode>&);
private:
	Poco::Data::Session*		m_session_ptr;
};
#endif

