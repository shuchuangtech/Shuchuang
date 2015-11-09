#include "Poco/Util/TimerTask.h"
#include "Poco/AutoPtr.h"
class CMyTaskHandler : public Poco::Util::TimerTask
{
public:
	typedef Poco::AutoPtr<CMyTaskHandler> Ptr;
	CMyTaskHandler();
	~CMyTaskHandler();
	void setTaskInfo(int id);
	void run();
private:
	int m_id;
	int m_repeat;
};

