#ifndef __MY_LOGGER_H__
#define __MY_LOGGER_H__
#include "Poco/Logger.h"
class CMyLogger
{
public:
	CMyLogger()
	{
	}
	~CMyLogger()
	{
	}
	void release()
	{
	}
	void information(char* buf)
	{
		m_logger.information(buf);
	}
private:
	Poco::Logger&	m_logger = Poco::Logger::get("Logger");
};
#endif

