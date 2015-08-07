#include "Common/ConfigManager.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Common/PrintLog.h"
CConfigManager::CConfigManager()
{
	m_path = "";
}

CConfigManager::~CConfigManager()
{
}

bool CConfigManager::init(const std::string path)
{
	File dir(path);
	if(dir.exists() && dir.isDirectory())
	{
		m_path = path + "/" + "global.conf";
		File file(m_path);
		if(file.exists())
		{
			try
			{
				m_config = new Util::JSONConfiguration(m_path);
			}
			catch(Exception& e)
			{
				warnf("%s, %d: Load config file error.", __FILE__, __LINE__);
			}
		}
		else
		{
			m_config = new Util::JSONConfiguration;
		}
		infof("%s, %d: ConfigManager init successfully, directory path %s", __FILE__, __LINE__, path.c_str());
		return true;
	}
	else
	{
		m_path = "";
		warnf("%s, %d: ConfigManager init failed", __FILE__, __LINE__);
		return false;
	}
}

bool CConfigManager::getConfig(const std::string configName, JSON::Object::Ptr& config)
{
	if(m_config.isNull() || configName.empty())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	if(m_config->has(configName))
	{
		std::string raw = "";
		raw = m_config->getRawString(configName, raw);
		JSON::Parser parser;
		try
		{
			Dynamic::Var var = parser.parse(raw);
			config = var.extract<JSON::Object::Ptr>();
		}
		catch(Exception& e)
		{
			errorf("%s, %d: Config %s is not JSON format.", __FILE__, __LINE__, configName.c_str());
			config = NULL;
			return false;
		}
	}
	else
	{
		config = NULL;
		warnf("%s, %d: Config %s not exists.", __FILE__, __LINE__, configName.c_str());
	}
	return true;
}

bool CConfigManager::setConfig(const std::string configName, JSON::Object::Ptr config)
{
	if(m_config.isNull() || configName.empty() || config.isNull())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	DynamicStruct ds = *config;
	m_config->setString(configName, ds.toString());
	FileOutputStream fos(m_path);
	m_config->save(fos);
	fos.close();
	return true;
}

