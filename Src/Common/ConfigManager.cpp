#include "Common/ConfigManager.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Common/PrintLog.h"
using namespace Poco;
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
		Util::JSONConfiguration jsonConf;
		if(file.exists())
		{
			try
			{
				jsonConf.load(m_path);
			}
			catch(Exception& e)
			{
				warnf("%s, %d: Load config file error.", __FILE__, __LINE__);
				return false;
			}
			if(!jsonConf.has("root"))
			{
				warnf("%s, %d: Config doesn't has root node.", __FILE__, __LINE__);
				return false;
			}
			else {
				std::string str = jsonConf.getString("root");
				JSON::Parser parser;
				try
				{
					Dynamic::Var var = parser.parse(str);
					JSON::Object::Ptr config = var.extract<JSON::Object::Ptr>();
				}
				catch(Exception& e)
				{
					warnf("%s, %d: Global config is not JSON format.", __FILE__, __LINE__);
					return false;
				}
				return true;
			}
		}
		else
		{
			warnf("%s, %d: Config file not exists.", __FILE__, __LINE__);
			return false;
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

bool CConfigManager::resetConfig()
{
	JSON::Object::Ptr pReset;
	getConfig("Reset", pReset);
	if(!pReset.isNull() && pReset->has("Config"))
	{
		std::string backupConfig = pReset->getValue<std::string>("Config");
		File file(backupConfig);
		file.copyTo(m_path);
		infof("%s, %d: Config manager reset config successfully.", __FILE__, __LINE__);
		return true;
	}
	else
	{
		warnf("%s, %d: Config manager reset config failed.", __FILE__, __LINE__);
		return false;
	}
}

bool CConfigManager::getAllConfig(JSON::Object::Ptr& config)
{
	Mutex::ScopedLock lock(m_mutex);
	Util::JSONConfiguration jsonConf(m_path);
	std::string str = jsonConf.getString("root");
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(str);
	config = var.extract<JSON::Object::Ptr>();
	return true;
}

bool CConfigManager::getConfig(const std::string configName, JSON::Object::Ptr& config)
{
	if(configName.empty())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	Util::JSONConfiguration jsonConf(m_path);
	std::string str = jsonConf.getString("root");
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(str);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj->has(configName) && pObj->isObject(configName))
	{
		config = pObj->getObject(configName);
	}
	else
	{
		config = NULL;
		warnf("%s, %d: Config %s not exists or is not object.", __FILE__, __LINE__, configName.c_str());
	}
	return true;
}

bool CConfigManager::getConfig(const std::string configName, JSON::Array::Ptr& config)
{
	if(configName.empty())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	Util::JSONConfiguration jsonConf(m_path);
	std::string str = jsonConf.getString("root");
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(str);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj->has(configName) && pObj->isArray(configName))
	{
		config = pObj->getArray(configName);
	}
	else
	{
		config = NULL;
		warnf("%s, %d: Config %s not exists or is not array.", __FILE__, __LINE__, configName.c_str());
	}
	return true;
}

bool CConfigManager::setConfig(const std::string configName, JSON::Object::Ptr config)
{
	if(configName.empty() || config.isNull())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	Util::JSONConfiguration jsonConf(m_path);
	std::string str = jsonConf.getString("root");
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(str);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj->has(configName))
	{
		if(pObj->isObject(configName))
		{
			pObj->remove(configName);
			pObj->set(configName, config);
		}
		else
		{
			return false;
		}
	}
	else
	{
		pObj->set(configName, config);
	}
	JSON::Object::Ptr pRoot = new JSON::Object;
	pRoot->set("root", pObj);
	FileOutputStream fos(m_path);
	Util::JSONConfiguration saveJson(pRoot);
	saveJson.save(fos);
	fos.close();
	return true;
}

bool CConfigManager::setConfig(const std::string configName, JSON::Array::Ptr config)
{
	if(configName.empty() || config.isNull())
		return false;
	Mutex::ScopedLock lock(m_mutex);
	Util::JSONConfiguration jsonConf(m_path);
	std::string str = jsonConf.getString("root");
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(str);
	JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
	if(pObj->has(configName))
	{
		if(pObj->isArray(configName))
		{
			pObj->remove(configName);
			pObj->set(configName, config);
		}
		else
		{
			return false;
		}
	}
	else
	{
		pObj->set(configName, config);
	}
	JSON::Object::Ptr pRoot = new JSON::Object;
	pRoot->set("root", pObj);
	FileOutputStream fos(m_path);
	Util::JSONConfiguration saveJson(pRoot);
	saveJson.save(fos);
	fos.close();
	return true;
}

