#include "TransmitServer/UpdateManager/UpdateManager.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Array.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Common/PrintLog.h"
#include "Common/ConfigManager.h"
#include "Common/RPCDef.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
using namespace Poco;
CUpdateManager::CUpdateManager()
{
}

CUpdateManager::~CUpdateManager()
{
}

bool CUpdateManager::init()
{
	CConfigManager* configManager = CConfigManager::instance();
	JSON::Object::Ptr pConfig;
	configManager->getConfig("Update", pConfig);
	if(pConfig.isNull() || !pConfig->has("DirPath"))
	{
		warnf("%s, %d: UpdateManager init failed, config 'Update' not exists.", __FILE__, __LINE__);
		return false;
	}
	std::string update_dir = pConfig->getValue<std::string>("DirPath");
	m_update_path.assign(update_dir);
	if(!m_update_path.isDirectory())
	{
		warnf("%s, %d: UpdateManager init failed, %s is not a directory.", __FILE__, __LINE__, update_dir.c_str());
		return false;
	}
	return true;
}

bool CUpdateManager::checkUpdate(JSON::Object::Ptr& param, std::string& detail)
{
	if(!m_update_path.isDirectory())
	{
		warnf("%s, %d: Update manager check update error[update dir not exists], please init first.", __FILE__, __LINE__);
	}
	if(param.isNull() || !param->has(UPDATE_TYPE_STR))
	{
		detail = "450";
		return false;
	}
	std::string devType = param->getValue<std::string>(UPDATE_TYPE_STR);
	Path path(m_update_path);
	path.append(devType);
	path.append(Path::separator());
	if(!path.isDirectory())
	{
		detail = "451";
		return false;
	}
	path.append("Info");
	if(!path.isFile())
	{
		detail = "452";
		warnf("%s, %d: UpdateManager checkUpdate failed, %s update info file not exists.", __FILE__, __LINE__, devType.c_str());
		return false;
	}
	Util::JSONConfiguration updateConf;
	try
	{
		updateConf.load(path.toString());
	}
	catch(Exception& e)
	{
		detail = "453";
		warnf("%s, %d: Load JSON Configuration file failed.", __FILE__, __LINE__);
		return false;
	}
	std::string version = updateConf.getString(UPDATE_VERSION_STR);
	std::string buildtime = updateConf.getString(UPDATE_BUILDTIME_STR);
	JSON::Object::Ptr pUpdate = new JSON::Object;
	pUpdate->set(UPDATE_VERSION_STR, version);
	pUpdate->set(UPDATE_BUILDTIME_STR, buildtime);
	std::string newfeature = updateConf.getRawString(UPDATE_NEWFEATURE_STR);
	JSON::Parser parser;
	Dynamic::Var var = parser.parse(newfeature);
	JSON::Array::Ptr pNew = var.extract<JSON::Array::Ptr>();
	pUpdate->set(UPDATE_NEWFEATURE_STR, pNew);
	param->set("update", pUpdate);
	return true;
}

