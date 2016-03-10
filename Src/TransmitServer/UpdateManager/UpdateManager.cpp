#include "TransmitServer/UpdateManager/UpdateManager.h"
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
	FileInputStream fis(path.toString());
	char t_buf[128] = {0, };
	JSON::Object::Ptr pUpdate = new JSON::Object;
	//version
	fis.getline(t_buf, 128);
	std::string version = t_buf;
	pUpdate->set(UPDATE_VERSION_STR, version);
	memset(t_buf, 0, 128);
	//buildtime
	fis.getline(t_buf, 128);
	std::string timestamp = t_buf;
	pUpdate->set(UPDATE_BUILDTIME_STR, timestamp);
	memset(t_buf, 0, 128);
	//checksum
	fis.getline(t_buf, 128);
	std::string checksum = t_buf;
	pUpdate->set(UPDATE_CHECKSUM_STR, checksum);
	JSON::Array::Ptr pNew = new JSON::Array;
	while(1)
	{
		memset(t_buf, 0, 128);
		fis.getline(t_buf, 128);
		if(!fis.eof())
		{
			std::string newfeature = t_buf;
			pNew->add(newfeature);
		}
		else
		{
			break;
		}
	}
	fis.close();
	pUpdate->set(UPDATE_NEWFEATURE_STR, pNew);
	param->set("update", pUpdate);
	return true;
}

