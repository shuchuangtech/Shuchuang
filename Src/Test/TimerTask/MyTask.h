#ifndef __MYTASK_H__
#define __MYTASK_H__
#include "Poco/Util/TimerTask.h"
class MyTask : public Poco::Util::TimerTask
{
public:
	MyTask();
	~MyTask();
	void setId(int id);
	void run();
private:
	int		m_id;
	int		m_repeat;
};
#endif

