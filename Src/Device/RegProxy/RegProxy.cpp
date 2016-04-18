#include "Device/RegProxy.h"
#include "Device/Component/DeviceController.h"
#include "Common/ConfigManager.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Timespan.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Observer.h"
#include "Poco/MD5Engine.h"
using namespace Poco;
using namespace Poco::Net;
CRegProxy::CRegProxy()
{
	m_started = false;
	m_checkPeriod = 30 * 1000 * 1000;	//30s
	m_keepAliveTimeout = 2 * 60 * 1000 * 1000;	//2 minutes
	m_lastCheckTime = 0;
	m_sock = 0;
	m_ssl_sock = 0;
	m_token = "";
	m_uuid = "SC0000000000";
}

CRegProxy::~CRegProxy()
{
	if(m_sock != 0)
	{
		m_sock->close();
		delete m_sock;
		m_sock = 0;
	}
	if(m_ssl_sock != 0)
	{
		m_ssl_sock->close();
		delete m_ssl_sock;
		m_ssl_sock = 0;
	}
}

void CRegProxy::handleMessage(MessageNotification* pNf)
{
	MessageNotification::Ptr pNoti(pNf);
	if(pNoti->getName() == "SystemWillReboot")
	{
		infof("%s, %d: RegProxy receive SystemWillReboot notification, now disconnect from server.", __FILE__, __LINE__);
		stop();
		dealError(PLAIN_SOCKET);
	}
	else if(pNoti->getName() == "DeviceBindMobile")
	{
		JSON::Object::Ptr pParam = pNoti->getParam();
		if(!pParam.isNull())
		{
			DynamicStruct dp = *pParam;
			std::string mobile = pParam->getValue<std::string>(REG_MOBILETOKEN_STR);
			std::string installation = pParam->getValue<std::string>(REG_INSTALLATIONID_STR);
			DynamicStruct ds;
			ds[KEY_TYPE_STR] = TYPE_REQUEST_STR;
			char action[32] = {0, };
			snprintf(action, 32, "%s.%s", COMPONENT_SERVER_STR, SERVER_METHOD_BIND);
			ds[KEY_ACTION_STR] = action;
			DynamicStruct param;
			param[REG_UUID_STR] = m_uuid;
			param[REG_MOBILETOKEN_STR] = mobile;
			param[REG_INSTALLATIONID_STR] = installation;
			ds[KEY_PARAM_STR] = param;
			if(m_sock->sendBytes(ds.toString().c_str(), ds.toString().length()) > 0)
			{
				infof("%s, %d: Send mobile bind request[%s].", __FILE__, __LINE__, ds.toString().c_str());
			}
		}
	}
	else if(pNoti->getName() == "DeviceUnbindMobile")
	{
		DynamicStruct ds;
		ds[KEY_TYPE_STR] = TYPE_REQUEST_STR;
		char action[32] = {0, };
		snprintf(action, 32, "%s.%s", COMPONENT_SERVER_STR, SERVER_METHOD_UNBIND);
		ds[KEY_ACTION_STR] = action;
		DynamicStruct param;
		param[REG_UUID_STR] = m_uuid;
		ds[KEY_PARAM_STR] = param;
		if(m_sock->sendBytes(ds.toString().c_str(), ds.toString().length()) > 0)
		{
			infof("%s, %d: Send mobile unbind request[%s].", __FILE__, __LINE__, ds.toString().c_str());
		}
	}
}

void CRegProxy::handleNf(RequestNotification* pNf)
{
	RequestNotification::Ptr p(pNf);
	if(p->getID() == (UInt64)m_sock)
	{
		if(m_sock == 0)
		{
			errorf("%s, %d: Tcp connection unestablished.", __FILE__, __LINE__);
			return;
		}
		JSON::Object::Ptr response = p->getResponse();
		if(!response.isNull())
		{
			DynamicStruct ds = *response;
			std::string ds_str = ds.toString();
			if(m_sock->sendBytes(ds_str.c_str(), ds_str.length()) > 0)
			{
				debugf("%s, %d: Receive notification and send:%s.", __FILE__, __LINE__, ds_str.c_str());
			}
		}
	}
}

void CRegProxy::start()
{
	if(m_started)
	{
		warnf("%s, %d: RegProxy already started.", __FILE__, __LINE__);
		return;
	}
	m_rpc = CRPCServer::instance();
	if(m_rpc == NULL)
	{
		errorf("%s, %d: Get RPCServer instance failed.", __FILE__, __LINE__);
		return;
	}
	CConfigManager* config = CConfigManager::instance();
	JSON::Object::Ptr pConfig = NULL;
	config->getConfig("RegProxy", pConfig);
	if(pConfig.isNull())
	{
		errorf("%s, %d: Please set default RegProxy config first.", __FILE__, __LINE__);
		return;
	}
	else
	{
		m_ssl_host = pConfig->get("host").extract<std::string>();
		m_reg_host = m_ssl_host;
		m_ssl_port = (UInt16)pConfig->get("ssl_port").extract<int>();
		m_reg_port = (UInt16)pConfig->get("reg_port").extract<int>();
	}
	JSON::Object::Ptr pDevInfo = NULL;
	config->getConfig("DeviceInfo", pDevInfo);
	m_uuid = pDevInfo->getValue<std::string>("uuid");
	Observer<CRegProxy, RequestNotification> observer(*this, &CRegProxy::handleNf);
	m_rpc->addObserver(observer);
	Observer<CRegProxy, MessageNotification> ob(*this, &CRegProxy::handleMessage);
	NotificationCenter::defaultCenter().addObserver(ob);
	CDeviceController::instance()->errOn();
	m_timer.setPeriodicInterval(0);
	m_timer.setStartInterval(500);
	m_started = true;
	TimerCallback<CRegProxy> callback(*this, &CRegProxy::onTimer);
	m_timer.start(callback);
	infof("%s, %d: RegProxy start succefully.", __FILE__, __LINE__);
}

void CRegProxy::stop()
{
	if(!m_started)
	{
		return;
	}
	Observer<CRegProxy, RequestNotification> observer(*this, &CRegProxy::handleNf);
	m_rpc->removeObserver(observer);
	m_started = false;
	m_timer.stop();
	infof("%s, %d: RegProxy stopped.", __FILE__, __LINE__);
}

bool CRegProxy::getRegisterToken()
{
	if(m_ssl_sock != 0)
	{
		m_ssl_sock->close();
		delete m_ssl_sock;
	}
	SocketAddress saddr(m_ssl_host, m_ssl_port);
	try
	{
		infof("%s, %d: Connecting to %s.", __FILE__, __LINE__, saddr.toString().c_str());
		Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
		m_ssl_sock = new SecureStreamSocket(pContext);
		m_ssl_sock->connect(saddr, Timespan(3, 0));
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Connect failed %s.", __FILE__, __LINE__, e.message().c_str());
		dealError(SECURE_SOCKET);
		return false;
	}
	infof("%s, %d: Connect to the server successfully.", __FILE__, __LINE__);
	char buf[512] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_GETTOKEN);
	tracef("%s, %d: Send gettoken buf: %s.", __FILE__, __LINE__, buf);
	m_ssl_sock->setSendTimeout(Timespan(10, 0));
	try
	{
		if(m_ssl_sock->sendBytes(buf, sizeof(buf)) > 0)
		{
			infof("%s, %d: Send get token message successfully.", __FILE__, __LINE__);
		}
		else
		{
			dealError(SECURE_SOCKET);
			errorf("%s, %d: Send get token message failed.", __FILE__, __LINE__);
			return false;
		}
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Send get token message exception[%s].", __FILE__, __LINE__, e.message().c_str());
		dealError(SECURE_SOCKET);
		return false;
	}
	memset(buf, 0 ,sizeof(buf));
	m_ssl_sock->setReceiveTimeout(Timespan(10, 0));
	try
	{
		if(m_ssl_sock->receiveBytes(buf, (UInt16)sizeof(buf)) > 0)
		{
			tracef("%s, %d: Receive gettoken response:%s.", __FILE__, __LINE__, buf);
			m_ssl_sock->close();
			delete m_ssl_sock;
			m_ssl_sock = NULL;
			JSON::Parser parser;
			Dynamic::Var var;
			try
			{
				var = parser.parse(buf);
			}
			catch(Exception& e)
			{
				errorf("%s, %d: Get token response not json format.", __FILE__, __LINE__);
				return false;
			}
			JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
			if(pObj.isNull() || !pObj->has(KEY_TYPE_STR) || !pObj->has(KEY_RESULT_STR) || !pObj->has(KEY_PARAM_STR))
			{
				warnf("%s, %d: Receive message miss type, result, or param.", __FILE__, __LINE__);
				return false;
			}
			if(pObj->getValue<std::string>(KEY_TYPE_STR) != TYPE_RESPONSE_STR)
			{
				warnf("%s, %d: Receive message type is not response.", __FILE__, __LINE__);
				return false;
			}
			if(pObj->getValue<std::string>(KEY_RESULT_STR) != RESULT_GOOD_STR)
			{
				warnf("%s, %d: Server request failed.", __FILE__, __LINE__);
				return false;
			}
			JSON::Object::Ptr pParam = pObj->getObject(KEY_PARAM_STR);
			if(pParam.isNull() || !pParam->has(REG_UUID_STR) || !pParam->has(REG_KEY_STR) || !pParam->has(REG_TIMESTAMP_STR) || !pParam->has(REG_TOKEN_STR))
			{
				warnf("%s, %d: Param miss uuid, key, timestamp, or token.", __FILE__, __LINE__);
				return false;
			}
			std::string uuid = pParam->getValue<std::string>(REG_UUID_STR);
			if(uuid != m_uuid)
			{
				warnf("%s, %d: Server response uuid error.", __FILE__, __LINE__);
				return false;
			}
			std::string recvkey = pParam->getValue<std::string>(REG_KEY_STR);
			std::string timestamp = pParam->getValue<std::string>(REG_TIMESTAMP_STR);
			std::string key = "alpha2015";
			key += timestamp;
			MD5Engine md5;
			md5.update(key);
			const DigestEngine::Digest& digest = md5.digest();
			std::string md5key = DigestEngine::digestToHex(digest);
			if(recvkey != md5key)
			{
				warnf("%s, %d: Verify server key failed.", __FILE__, __LINE__);
				return false;
			}
			m_token = pParam->getValue<std::string>(REG_TOKEN_STR);
			infof("%s, %d: Get register token successfully.", __FILE__, __LINE__);
			return true;
		}
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Receive token timeout[%s].", __FILE__, __LINE__, e.message().c_str());
		dealError(SECURE_SOCKET);
		return false;
	}
	return false;
}

bool CRegProxy::registerToServer()
{
	if(!getRegisterToken())
	{
		warnf("%s, %d: Get register token failed.", __FILE__, __LINE__);
		return false;
	}
	SocketAddress saddr(m_reg_host, m_reg_port);
	try
	{
		infof("%s, %d: Connecting to %s.", __FILE__, __LINE__, saddr.toString().c_str());
		if(m_sock != 0)
		{
			m_sock->close();
			delete m_sock;
			m_sock = 0;
		}
		Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
		m_sock = new SecureStreamSocket(pContext);
		m_sock->connect(saddr, Timespan(10, 0));
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Connect failed[%s].", __FILE__, __LINE__, e.message().c_str());
		dealError(PLAIN_SOCKET);
		return false;
	}
	infof("%s, %d: Connect to the server successfully.", __FILE__, __LINE__);
	char buf[512] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_REGISTER);
	tracef("%s, %d: Send register buf: %s.", __FILE__, __LINE__, buf);
	m_sock->setSendTimeout(Timespan(10, 0));
	try
	{
		if(m_sock->sendBytes(buf, sizeof(buf)) > 0)
		{
			infof("%s, %d: Register information sent.", __FILE__, __LINE__);
		}
		else
		{
			dealError(PLAIN_SOCKET);
			errorf("%s, %d: Send register message failed.", __FILE__, __LINE__);
			return false;
		}
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Send register message exception[%s].", __FILE__, __LINE__, e.message().c_str());
		dealError(PLAIN_SOCKET);
		return false;
	}
	memset(buf, 0, sizeof(buf));
	m_sock->setReceiveTimeout(Timespan(10, 0));
	try
	{
		if(m_sock->receiveBytes(buf, sizeof(buf)) > 0)
		{
			tracef("%s, %d: Receive register response: %s.", __FILE__, __LINE__, buf);
			JSON::Parser parser;
			Dynamic::Var var; 
			try
			{
				var = parser.parse(buf);
			}
			catch(Exception& e)
			{
				errorf("%s, %d: Register response not json format.", __FILE__, __LINE__);
				dealError(PLAIN_SOCKET);
				return false;
			}
			JSON::Object::Ptr pObj = var.extract<JSON::Object::Ptr>();
			if(pObj.isNull() || !pObj->has("result") || pObj->getValue<std::string>("result") != "good")
			{
				warnf("%s, %d: Register failed.", __FILE__, __LINE__);
				dealError(PLAIN_SOCKET);
				return false;
			}
			else
			{
				infof("%s, %d: Register successfully.", __FILE__, __LINE__);
				return true;
			}
		}
		else
		{
			errorf("%s, %d: Receive register response error.", __FILE__, __LINE__);
			dealError(PLAIN_SOCKET);
			return false;
		}
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Receive register response exception[%s].", __FILE__, __LINE__, e.message().c_str());
		dealError(PLAIN_SOCKET);
		return false;
	}
	dealError(PLAIN_SOCKET);
	return false;
}

void CRegProxy::createPacket(char* buf, UInt16 size, REQUEST_ACTION ra)
{
	DynamicStruct ds;
	ds[KEY_TYPE_STR] = TYPE_REQUEST_STR;
	CConfigManager* config = CConfigManager::instance();
	switch(ra)
	{
		case ACTION_GETTOKEN :
		{
			DynamicStruct param;
			ds[KEY_ACTION_STR] = "server.token";
			JSON::Object::Ptr pDevInfo;
			config->getConfig("DeviceInfo", pDevInfo);
			JSON::Object::Ptr pAPNS;
			config->getConfig("APNS", pAPNS);
			std::string mobile = pAPNS->getValue<std::string>("MobileToken");
			std::string installation = pAPNS->getValue<std::string>("InstallationId");
			param[REG_MOBILETOKEN_STR] = mobile;
			param[REG_INSTALLATIONID_STR] = installation;
			param[REG_DEV_NAME_STR] = pDevInfo->getValue<std::string>("name");
			param[REG_DEV_TYPE_STR] = pDevInfo->getValue<std::string>("type");
			param[REG_DEV_MANU_STR] = pDevInfo->getValue<std::string>("manufacture");
			param[REG_UUID_STR] = m_uuid;
			ds[KEY_PARAM_STR] = param;
			break;
		}
		case ACTION_REGISTER :
		{
			ds[KEY_ACTION_STR] = "server.register";
			DynamicStruct param;
			param[REG_TOKEN_STR] = m_token;
			param[REG_UUID_STR] = m_uuid;
			ds[KEY_PARAM_STR] = param;
			break;
		}
		case ACTION_KEEPALIVE :
		{
			ds[KEY_ACTION_STR] = "server.keepalive";
			DynamicStruct param;
			param[REG_UUID_STR] = m_uuid;
			ds[KEY_PARAM_STR] = param;
			break;
		}
		default :
			break;
	}
	snprintf(buf, size - 1, "%s", ds.toString().c_str());
}

bool CRegProxy::parseAction(std::string& opt, std::string& component, std::string& method)
{
	std::string::size_type pos;
	pos = opt.find(".");
	if(pos == std::string::npos)
	{
		warnf("%s, %d: Action parse failed.", __FILE__, __LINE__);
		return false;
	}
	component = opt.substr(0, pos);
	method = opt.substr(pos + 1, opt.length() - pos - 1);
	return true;
}

void CRegProxy::onTimer(Timer& timer)
{
	char buf[512] = {0, };
	while(m_started)
	{
		if(m_sock == 0)
		{
			warnf("%s, %d: Tcp disconnected.", __FILE__, __LINE__);
			if(registerToServer())
			{
				Timestamp t;
				m_lastCheckTime =  t;
				CDeviceController::instance()->errOff();
			}
			else
			{
				Thread::sleep(5 * 1000);
			}
		}
		else
		{
			//receive connection request
			if(m_sock->poll(Timespan(5, 0), Socket::SELECT_READ|Socket::SELECT_ERROR) > 0)
			{
				memset(buf, 0, sizeof(buf));
				m_sock->setReceiveTimeout(Timespan(10, 0));
				int ret = 0;
				try
				{
					ret = m_sock->receiveBytes(buf, sizeof(buf));
				}
				catch(Exception& e)
				{
					warnf("%s, %d: Reg socket receive exception[%s].", __FILE__, __LINE__, e.message().c_str());
				}
				if(ret <= 0 )
				{
					warnf("%s, %d: Receive error.", __FILE__, __LINE__);
					CDeviceController::instance()->errOn();
					dealError(PLAIN_SOCKET);
					continue;
				}
				std::string request(buf);
				tracef("%s, %d: Receive from server:%s.", __FILE__, __LINE__, buf);
				JSON::Object::Ptr nil = NULL;
				m_rpc->addRequest(new RequestNotification((UInt64)m_sock, request, nil));
			}
			Timestamp t;
			if(t - m_lastCheckTime >= m_checkPeriod)
			{
				if(sendKeepAlive())
				{
					m_lastCheckTime = t;
				}
				else
				{
					if( t - m_lastCheckTime > m_keepAliveTimeout )
					{
						warnf("%s, %d: Send keepalive timeout.", __FILE__, __LINE__);
						dealError(SECURE_SOCKET|PLAIN_SOCKET);
					}
				}
			}
		}
	}
}

bool CRegProxy::sendKeepAlive()
{
	char buf[512] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_KEEPALIVE);
	if(m_sock->sendBytes(buf, strlen(buf)) > 0 )
	{
		infof("%s, %d: KeepAlive successfully.", __FILE__, __LINE__);
		return true;
	}
	else
	{
		warnf("%s, %d: KeepAlive sent failed.", __FILE__, __LINE__);
		return false;
	}

}

void CRegProxy::dealError(ERROR_CHOICE choice)
{
	if((choice&SECURE_SOCKET) && m_ssl_sock != 0)
	{
		m_ssl_sock->close();
		delete m_ssl_sock;
		m_ssl_sock = 0;
	}
	if((choice&PLAIN_SOCKET) && m_sock != 0)
	{
		m_sock->close();
		delete m_sock;
		m_sock = 0;
	}
}

