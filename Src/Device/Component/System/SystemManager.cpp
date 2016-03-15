#include "Device/Component/System/SystemManager.h"
#include "Common/ConfigManager.h"
#include "Device/Util/UserRecord.h"
#include "Device/Util/OperationRecord.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/FTPClientSession.h"
#include "Poco/Util/JSONConfiguration.h"
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
	m_timer = NULL;
}

CSystemManager::~CSystemManager()
{
	if(m_timer != NULL)
		delete m_timer;
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
	if(param.isNull() || !param->has(SYSTEM_VERSION_STR))
	{
		detail = "464";
		return false;
	}
	m_update_version = param->getValue<std::string>(SYSTEM_VERSION_STR);
	TimerCallback<CSystemManager> updateTimerCallback(*this, &CSystemManager::handleUpdate);
	if(m_timer != NULL)
		delete m_timer;
	m_timer = new Timer;
	m_timer->start(updateTimerCallback);
	if(param->has(REG_TOKEN_STR))
	{
		param->remove(REG_TOKEN_STR);
	}
	return true;
}

bool CSystemManager::getDevVersion(Poco::JSON::Object::Ptr& param, std::string& detail)
{
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pUpdate;
	config->getConfig("Update", pUpdate);
	if(pUpdate.isNull() || !pUpdate->has("infoPath"))
	{
		detail = "901";
		return false;
	}
	std::string infoPath = pUpdate->getValue<std::string>("infoPath");
	File file(infoPath);
	if(!file.exists())
	{
		detail = "469";
		return false;
	}
	Util::JSONConfiguration info;
	std::string version;
	std::string buildtime;
	try
	{
		info.load(infoPath);
		version = info.getString(UPDATE_VERSION_STR);
		buildtime = info.getString(UPDATE_BUILDTIME_STR);
	}
	catch(Exception& e)
	{
		detail = "469";
		warnf("%s, %d: Load device info file failed, %s", __FILE__, __LINE__, e.message().c_str());
		return false;
	}
	if(param->has(REG_TOKEN_STR))
	{
		param->remove(REG_TOKEN_STR);
	}
	param->set(SYSTEM_VERSION_STR, version);
	param->set(SYSTEM_BUILDTIME_STR, buildtime);
	return true;
}

void CSystemManager::handleUpdate(Timer& timer)
{
	if(!m_updating)
	{
		return;
	}
	infof("%s, %d: SystemManager update handler is running.", __FILE__, __LINE__);
	//connect to ftp server
	CConfigManager *config = CConfigManager::instance();
	JSON::Object::Ptr pUpdate;
	config->getConfig("Update", pUpdate);
	if(pUpdate.isNull() || !pUpdate->has("server") || !pUpdate->has("username") || !pUpdate->has("username"))
	{
		m_updating = false;
		warnf("%s, %d: Start update failed, Update config not exists.", __FILE__, __LINE__);
		return;
	}
	std::string serverAddr = pUpdate->getValue<std::string>("server");
	std::string username = pUpdate->getValue<std::string>("username");
	std::string password = pUpdate->getValue<std::string>("password");
	infof("%s, %d: Try to connect update server[%s].", __FILE__, __LINE__, serverAddr.c_str());
	Net::FTPClientSession ftp(serverAddr, 21, username, password);
	if(!ftp.isLoggedIn())
	{
		m_updating = false;
		warnf("%s, %d: Update failed, ftp login failed.", __FILE__, __LINE__);
		ftp.close();
		return;
	}
	infof("%s, %d: Update ftp server connect successfully.", __FILE__, __LINE__);
	//get device type
	JSON::Object::Ptr pDevInfo;
	config->getConfig("DeviceInfo", pDevInfo);
	std::string devType = pDevInfo->getValue<std::string>("type");
	//download update info
	std::string infoPath = devType + "/Info";
	FileOutputStream ofsInfo("./Info_temp", std::ios::out|std::ios::trunc);
	infof("%s, %d: Downloading update info %s.", __FILE__, __LINE__, infoPath.c_str());
	try
	{
		std::istream& ist = ftp.beginDownload(infoPath);
		ofsInfo << ist.rdbuf();
	}
	catch(Exception& e)
	{
		m_updating = false;
		errorf("%s, %d: Download update info failed.");
		ofsInfo.close();
		ftp.endDownload();
		ftp.close();
		return;
	}
	ftp.endDownload();
	ofsInfo.close();
	infof("%s, %d: Update info download successfully.", __FILE__, __LINE__);
	//load update info
	Util::JSONConfiguration infoConf;
	std::string update_version;
	std::string update_buildtime;
	std::string update_checksum;
	try
	{
		infoConf.load("./Info_temp");
		update_version = infoConf.getString(UPDATE_VERSION_STR);
		update_buildtime = infoConf.getString(UPDATE_BUILDTIME_STR);
		update_checksum = infoConf.getString(UPDATE_CHECKSUM_STR);
	}
	catch(Exception& e)
	{
		m_updating = false;
		errorf("%s, %d: Load update info file failed.");
		ftp.close();
		return;
	}
	//verify buildtime
	std::string buildtime(getMKTIME());
	std::string timeFmt = "%Y-%m-%d %H:%M:%S %Z";
	DateTime buildDate;
	DateTime updateBuildDate;
	int timeZoneDiff = 0;
	DateTimeParser::parse(timeFmt, buildtime, buildDate, timeZoneDiff);
	DateTimeParser::parse(timeFmt, update_buildtime, updateBuildDate, timeZoneDiff);

	if(buildDate > updateBuildDate)
	{
		m_updating = false;
		warnf("%s, %d: Running app build time newer than update.", __FILE__, __LINE__);
		ftp.close();
		return;
	}
	//download update file
	infof("%s, %d: Update information[version:%s, buildtime:%s].", __FILE__, __LINE__, update_version.c_str(), update_buildtime.c_str());
	FileOutputStream ofs("./DeviceTest_temp", std::ios::out|std::ios::trunc|std::ios::binary);
	std::string path = devType + "/" +  m_update_version + "/DeviceTest";
	infof("%s, %d: Downloading update file: %s.", __FILE__, __LINE__, path.c_str());
	try
	{
		std::istream& ist = ftp.beginDownload(path);
		ofs << ist.rdbuf();
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Update file download failed, %s.", __FILE__, __LINE__, e.message().c_str());
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
	infof("%s, %d: Checking update file MD5 checksum.", __FILE__, __LINE__);
	if(getFileMD5("./DeviceTest_temp") != update_checksum)
	{
		warnf("%s, %d: Update file MD5 not in accordance.", __FILE__, __LINE__);
		m_updating = false;
		return;
	}
	infof("%s, %d: Replace info file.", __FILE__, __LINE__);
	File infoFile("./Info_temp");
	std::string localInfoPath = pUpdate->getValue<std::string>("infoPath");
	infoFile.copyTo(localInfoPath);
	File file("./DeviceTest_temp");
	file.setExecutable(true);
	infof("%s, %d: Delete download info file.", __FILE__, __LINE__);
	infoFile.remove();
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

