#include "Device/Util/OperationRecord.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Statement.h"
#include "Poco/Timestamp.h"
#include "Poco/Timespan.h"
#include <stdio.h>
#include <stdlib.h>
#include "Poco/Thread.h"
#include "Poco/Types.h"
COperationRecord* op;
int repeat = 0;
void insertSingle()
{
	Poco::Int64 totalMilliseconds = 0;
	for(int i = 0; i < 10; i++)
	{
		Poco::Timestamp now;
		OperationRecordNode orn;
		orn.timestamp = now.epochMicroseconds();
		orn.operation = i%2;
		char num[8];
		snprintf(num, 7 , "%d", i + 100);
		orn.username = "test";
		orn.username += num;
		orn.schema = (i + 1)%2;
		Poco::Timestamp begin2;
		op->addRecord(orn);
		Poco::Timestamp end2;
		Poco::Timespan diff2 = end2 - begin2;
		totalMilliseconds += diff2.totalMilliseconds();
	}
	printf("individually insert 10 data average use %llu milliseconds\n", totalMilliseconds/10);
}

void insertByVector()
{	
	std::vector<OperationRecordNode> data_set;
	for(int i = 0; i < repeat; i++)
	{
		Poco::Timestamp now;
		OperationRecordNode orn;
		orn.timestamp = now.epochMicroseconds();
		orn.operation = i%2;
		char num[8];
		snprintf(num, 7 , "%d", i);
		orn.username = "test";
		orn.username += num;
		orn.schema = (i + 1)%2;
		data_set.push_back(orn);
		Poco::Thread::sleep(1);
	}
	Poco::Timestamp begin;
	int ret = op->addRecord(data_set);
	Poco::Timestamp end;
	Poco::Timespan diff = end - begin;
	printf("insert %d data using total %llu milliseconds\n", ret, diff.totalMilliseconds());
}

void insertByTransaction()
{
	Poco::Data::Session session("SQLite", "/home/hj/Dev_Env/Shuchuang/testinsert2.db");
	Poco::Data::Statement screate(session);
	screate << "CREATE TABLE IF NOT EXISTS `Operation` ("
				<< "`Timestamp` BIGINT PRIMARY KEY,"
				<< "`Operation` TINYINT,"
				<< "`Username` VARCHAR(64),"
				<< "`Schema` TINYINT)";
	screate.execute();

	Poco::Int64 totalMilliseconds = 0;
	std::vector<OperationRecordNode> data_set;
	for(int i = 0; i < repeat; i++)
	{
		Poco::Timestamp now;
		OperationRecordNode orn;
		orn.timestamp = now.epochMicroseconds();
		orn.operation = i%2;
		char num[8];
		snprintf(num, 7 , "%d", i);
		orn.username = "test";
		orn.username += num;
		orn.schema = (i + 1)%2;
		data_set.push_back(orn);
		Poco::Thread::sleep(1);
	}
	std::vector<OperationRecordNode>::iterator it;
	Poco::Data::Statement sinsert(session);
	Poco::Timestamp begin;
	session.begin();
	for(it = data_set.begin(); it != data_set.end(); it++)
	{
		sinsert.reset(session);
		sinsert << "INSERT INTO `Operation` (`Timestamp`, `Operation`, `Username`, `Schema`)"
			<< "VALUES(?, ?, ?, ?)",
			Poco::Data::Keywords::use(it->timestamp),
			Poco::Data::Keywords::use(it->operation),
			Poco::Data::Keywords::use(it->username),
			Poco::Data::Keywords::use(it->schema);
		sinsert.execute();
	}
	session.commit();
	Poco::Timestamp end;
	Poco::Timespan diff = end - begin;
	totalMilliseconds = diff.totalMilliseconds();
	printf("individually insert 10 data average use %llu milliseconds\n", totalMilliseconds/10);

}

int main(int argc, char** argv)
{
	if(argc > 1)
		repeat = atoi(argv[1]);
	op = COperationRecord::instance();
	op->init("/home/hj/Dev_Env/Shuchuang/testinsert.db");
	insertByVector();
	insertSingle();
	insertByTransaction();
	return 0;
}

