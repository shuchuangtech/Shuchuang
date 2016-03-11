#include "Device/Component/System/SystemManager.h"
#include "Common/ConfigManager.h"
#include "Device/Util/UserRecord.h"
#include "Device/Util/OperationRecord.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/FTPClientSession.h"
#include "Common/RPCDef.h"
#include "Common/PrintLog.h"
#include <iostream>
#include "Poco/FileStream.h"
#include "Poco/File.h"
#include <unistd.h>
#include <sys/reboot.h>
#include "Poco/NotificationCenter.h"
#include "Device/Notification/MessageNotification.h"
#include "Poco/Thread.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/MD5Engine.h"
extern const char* getMKTIME();
using namespace Poco;
CSystemManager::CSystemManager()
{
	m_updating = false;
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
	if(m_updating)
	{
		detail = "463";
		return false;
	}
	m_updating = true;
	if(param.isNull() || !param->has(SYSTEM_VERSION_STR) || !param->has(SYSTEM_TYPE_STR) || !param->has(SYSTEM_BUILDTIME_STR) || !param->has(SYSTEM_CHECKSUM_STR))
	{
		detail = "464";
		return false;
	}
	m_version = param->getValue<std::string>(SYSTEM_VERSION_STR);
	m_buildtime = param->getValue<std::string>(SYSTEM_BUILDTIME_STR);
	m_checksum = param->getValue<std::string>(SYSTEM_CHECKSUM_STR);
	m_type = param->getValue<std::string>(SYSTEM_TYPE_STR);
	//check build time
	std::string buildtime(getMKTIME());
	std::string timeFmt = "%Y-%m-%d %H:%M:%S %Z";
	DateTime buildDate;
	DateTime updateBuildDate;
	int timeZoneDiff = 0;
	DateTimeParser::parse(timeFmt, buildtime, buildDate, timeZoneDiff);
	DateTimeParser::parse(timeFmt, m_buildtime, updateBuildDate, timeZoneDiff);

	if(buildDate > updateBuildDate)
	{
		warnf("%s, %d: Running app build time newer than update.", __FILE__, __LINE__);
		detail = "468";
		return false;
	}


	CConfigManager *config = CConfigManager::instance();
	JSON::Object::Ptr pUpdate;
	config->getConfig("Update", pUpdate);
	if(pUpdate.isNull() || !pUpdate->has("server") || !pUpdate->has("username") || !pUpdate->has("username"))
	{
		warnf("%s, %d: Start update failed, Update config not exists.", __FILE__, __LINE__);
		detail = "465";
		return false;
	}
	JSON::Object::Ptr pDevInfo;
	config->getConfig("DeviceInfo", pDevInfo);
	std::string localDevtype = pDevInfo->getValue<std::string>("type");
	if(m_type != localDevtype)
	{
		detail = "466";
		warnf("%s, %d: Update device type error.", __FILE__, __LINE__);
		return false;
	}
	std::string serverAddr = pUpdate->getValue<std::string>("server");
	std::string username = pUpdate->getValue<std::string>("username");
	std::string password = pUpdate->getValue<std::string>("password");
	infof("%s, %d: Try to connect update server[%s].", __FILE__, __LINE__, serverAddr.c_str());
	Net::FTPClientSession ftp(serverAddr, 21, username, password);
	if(!ftp.isLoggedIn())
	{
		warnf("%s, %d: Update failed, ftp login failed.", __FILE__, __LINE__);
		ftp.close();
		detail = "467";
		return false;
	}
	ftp.close();
	infof("%s, %d: Update ftp server connect successfully, start update thread.", __FILE__, __LINE__);
	TimerCallback<CSystemManager> updateTimerCallback(*this, &CSystemManager::handleUpdate);
	m_timer.start(updateTimerCallback);
	return true;
}

void CSystemManager::handleUpdate(Timer& timer)
{
	if(!m_updating)
	{
		tracepoint();
		return;
	}
	infof("%s, %d: SystemManager update handler is running.", __FILE__, __LINE__);
	infof("%s, %d: Update information[version:%s, type:%s, buildtime:%s].", __FILE__, __LINE__, m_version.c_str(), m_type.c_str(), m_buildtime.c_str());
	CConfigManager *config = CConfigManager::instance();
	JSON::Object::Ptr pUpdate;
	config->getConfig("Update", pUpdate);
	std::string serverAddr = pUpdate->getValue<std::string>("server");
	std::string username = pUpdate->getValue<std::string>("username");
	std::string password = pUpdate->getValue<std::string>("password");
	
	Net::FTPClientSession ftp(serverAddr, 21, username, password);
	std::string path = m_type + "/" + m_version + "/DeviceTest";
	infof("%s, %d: Ready to download update file: %s.", __FILE__, __LINE__, path.c_str());
	FileOutputStream ofs("./DeviceTest_update", std::ios::out|std::ios::trunc|std::ios::binary);
	try
	{
		std::istream& ist = ftp.beginDownload(path);
		ofs << ist.rdbuf();
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Update download failed, %s.", __FILE__, __LINE__, e.message().c_str());
		ftp.endDownload();
		ofs.close();
		ftp.close();
		m_updating = false;
		return;
	}
	ofs.close();
	ftp.endDownload();
	ftp.close();
	infof("%s, %d: Update file download finished.", __FILE__, __LINE__);
	//checksum
	if(getFileMD5("./DeviceTest_update") != m_checksum)
	{
		warnf("%s, %d: Update file MD5 not in accordance.", __FILE__, __LINE__);
		return;
	}
	File file("./DeviceTest_update");
	file.setExecutable(true);
	std::string appPath = pUpdate->getValue<std::string>("appPath");
	file.copyTo(appPath);
	infof("%s, %d: Update file copy to %s.", __FILE__, __LINE__, appPath.c_str());
	file.remove();
	m_updating = false;
	rebootSystem();
}

std::string CSystemManager::getFileMD5(const std::string& filePath)
{
	FileInputStream fis(filePath, std::ios::in|std::ios::binary);
	int length = 0;
	char buf[8192] = {0, };
	MD5Engine md5;
	while(1)
	{
		memset(buf, 0, 8192);
		fis.read(buf, 8192);
		length = fis.gcount();
		md5.update(buf, length);
		if(fis.eof())
			break;
	}
	const DigestEngine::Digest& digest = md5.digest();
	std::string md5str(DigestEngine::digestToHex(digest));
	fis.close();
	return md5str;
}

void CSystemManager::rebootSystem()
{
	NotificationCenter& nc = NotificationCenter::defaultCenter();
	MessageNotification::Ptr pMessage = new MessageNotification;
	pMessage->setName("SystemWillReboot");
	nc.postNotification(pMessage);
	Thread::sleep(3000);
#ifndef __SC_ARM__
	warnf("%s, %d: X86 does not implement reboot.", __FILE__, __LINE__);
#else
	sync();
	reboot(RB_AUTOBOOT);
#endif
}

