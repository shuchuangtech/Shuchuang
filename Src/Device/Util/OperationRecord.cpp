#include "Device/Util/OperationRecord.h"
#include "Common/PrintLog.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Timespan.h"
using namespace Poco;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;
COperationRecord::COperationRecord()
{
	m_session_ptr = NULL;
}

COperationRecord::~COperationRecord()
{
	if(m_session_ptr != NULL)
	{
		m_session_ptr->close();
		delete m_session_ptr;
		m_session_ptr = NULL;
	}
}

bool COperationRecord::init(const std::string& dbPath)
{
	SQLite::Connector::registerConnector();
	try
	{
		m_session_ptr = new Session("SQLite", dbPath);
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Init OperationRecord with %s failed.", __FILE__, __LINE__, dbPath.c_str());
		return false;
	}
	infof("%s, %d: Init OperationRecord with %s successfully.", __FILE__, __LINE__, dbPath.c_str());
	try
	{
		Statement screate(*m_session_ptr);
		screate << "CREATE TABLE IF NOT EXISTS `Operation` ("
				<< "`Timestamp` BIGINT PRIMARY KEY,"
				<< "`Operation` TINYINT,"
				<< "`Username` VARCHAR(64),"
				<< "`Schema` TINYINT)";
		screate.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Create table Operation failed[%].", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	return true;
}

int COperationRecord::addRecord(OperationRecordNode& record)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	Statement sinsert(*m_session_ptr);
	sinsert << "INSERT INTO `Operation` (`Timestamp`, `Operation`, `Username`, `Schema`)"
			<< "VALUES(?, ?, ?, ?)",
		use(record.timestamp),
		use(record.operation),
		use(record.username),
		use(record.schema);
	int ret = 0;
	try
	{
		ret = sinsert.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Insert into Operation failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return -1;
	}
	return ret;
}

int COperationRecord::deleteRecordByDate(DateTime& date)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	Statement sdelete(*m_session_ptr);
	int ret = 0;
	Int64 t = date.timestamp().epochMicroseconds();
	sdelete << "DELETE FROM `Operation` WHERE `Timestamp`<?",
			use(t);
	try
	{
		ret = sdelete.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Delete from Operation failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return -1;
	}
	return ret;
}

int COperationRecord::getRecords(Timestamp& start, Timestamp& end, std::vector<OperationRecordNode>& data_set)
{
	if(m_session_ptr == NULL)
	{
		warnf("%s, %d: Please init database first.", __FILE__, __LINE__);
		return -1;
	}
	data_set.clear();
	Timestamp tstart = start;
	Int64 ststart = tstart.epochMicroseconds();
	Timestamp tend = end - 1;
	Int64 stend = tend.epochMicroseconds();
	Statement sselect(*m_session_ptr);
	sselect << "SELECT `Timestamp`, `Operation`, `Username`, `Schema` FROM `Operation` WHERE `Timestamp` BETWEEN ? AND ?",
			use(ststart),
			use(stend);
	int ret = 0;
	try
	{
		ret = sselect.execute();
	}
	catch(Exception& e)
	{
		warnf("%s, %d: Select from Operation failed[%s].", __FILE__, __LINE__, e.message().c_str());
		return -1;
	}
	RecordSet rs(sselect);
	bool more = rs.moveFirst();
	while(more)
	{
		OperationRecordNode op_node;
		std::size_t col = 0;
		op_node.timestamp = rs[col++].convert<Int64>();
		op_node.operation = rs[col++].convert<int>();
		op_node.username = rs[col++].convert<std::string>();
		op_node.schema = rs[col++].convert<int>();
		more = rs.moveNext();
		data_set.push_back(op_node);
	}
	return ret;
}

