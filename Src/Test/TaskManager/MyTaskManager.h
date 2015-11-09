#include "Poco/SingletonHolder.h"
#include "Poco/Util/Timer.h"
#include "MyTaskHandler.h"
class CMyTaskManager
{
public:
	CMyTaskManager();
	~CMyTaskManager();
	static CMyTaskManager* instance()
	{
		static Poco::SingletonHolder<CMyTaskManager> sh;
		return sh.get();
	}
	void addTask(int task_id);
	void removeTask(int task_id);
private:
	Poco::Util::Timer*	m_timer;
	std::map<int, CMyTaskHandler::Ptr>	m_map;
};

