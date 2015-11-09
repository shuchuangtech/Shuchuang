#include "Poco/SingletonHolder.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
class TestConfig
{
public:
	static TestConfig* instance()
	{
		static Poco::SingletonHolder<TestConfig> sh;
		return sh.get();
	}
	TestConfig();
	~TestConfig();
	bool setConfig(std::string configName, Poco::JSON::Object::Ptr& config);
	bool setConfig(std::string configName, Poco::JSON::Array::Ptr& config);
private:
	Poco::JSON::Object::Ptr	m_config;
};

