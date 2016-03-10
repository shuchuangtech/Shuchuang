#include "Device/Component/System/SystemManager.h"
#include "Common/ConfigManager.h"
#include "Device/Util/UserRecord.h"
#include "Device/Util/OperationRecord.h"
#include "Poco/JSON/Object.h"
using namespace Poco;
CSystemManager::CSystemManager()
{
}

CSystemManager::~CSystemManager()
{
}

bool CSystemManager::resetConfig(JSON::Object::Ptr& param, std::string& detail)
{
	CConfigManager* config = CConfigManager::instance();
	if(!config->resetConfig())
	{
		detail = "461";
		return false;
	}
	JSON::Object::Ptr pReset;
	config->getConfig("Reset", pReset);
	JSON::Object::Ptr pDataPath;
	config->getConfig("DataPath", pDataPath);
	std::string userDBPath = pDataPath->getValue<std::string>("User");
	std::string backupUserDBPath = pReset->getValue<std::string>("UserDB");
	CUserRecord* userRecord = CUserRecord::instance();
	if(!userRecord->resetUser(userDBPath, backupUserDBPath))
	{
		detail = "462";
		return false;
	}
	COperationRecord* opRecord = COperationRecord::instance();
	if(!opRecord->resetOpRecord())
	{
		detail = "463";
		return false;
	}
	return true;
}

bool CSystemManager::startUpdate(JSON::Object::Ptr& param, std::string& detail)
{
	return false;
}

