#include "Device/RegProxy.h"
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
	m_dev_name = m_uuid;
	m_dev_type = "default";
	m_manufacture = "Shuchuangtech";
}

CRegProxy::~CRegProxy()
{
	if(m_sock != 0)
	{
		m_sock->close();
		delete m_sock;
	}
	if(m_ssl_sock != 0)
	{
		m_ssl_sock->close();
		delete m_ssl_sock;
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
	pConfig = NULL;
	config->getConfig("DeviceInfo", pConfig);
	if(pConfig.isNull())
	{
		errorf("%s, %d: Please set default RegProxy config first.", __FILE__, __LINE__);
		return;
	}
	else
	{
		m_uuid = pConfig->getValue<std::string>("uuid");
		m_dev_name = pConfig->getValue<std::string>("name");
		m_dev_type = pConfig->getValue<std::string>("type");
		m_manufacture = pConfig->getValue<std::string>("manufacture");
	}
	Observer<CRegProxy, RequestNotification> observer(*this, &CRegProxy::handleNf);
	m_rpc->addObserver(observer);	
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
	memset(buf, 0 ,sizeof(buf));
	m_ssl_sock->setReceiveTimeout(Timespan(10, 0));
	try
	{
		if(m_ssl_sock->receiveBytes(buf, (UInt16)sizeof(buf)) > 0)
		{
			tracef("%s, %d: Receive gettoken response:%s.", __FILE__, __LINE__, buf);
			JSON::Parser parser;
			Dynamic::Var var = parser.parse(buf);
			JSON::Object::Ptr obj = var.extract<JSON::Object::Ptr>();
			DynamicStruct ds = *obj;
			if(!ds.contains(KEY_TYPE_STR) || !ds.contains(KEY_RESULT_STR) || !ds.contains(KEY_PARAM_STR))
			{
				warnf("%s, %d: Receive message miss type, result, or param.", __FILE__, __LINE__);
				return false;
			}
			if(ds[KEY_TYPE_STR].toString() != TYPE_RESPONSE_STR)
			{
				warnf("%s, %d: Receive message type is not response.", __FILE__, __LINE__);
				return false;
			}
			if(ds[KEY_RESULT_STR] != RESULT_GOOD_STR)
			{
				warnf("%s, %d: Server request failed.", __FILE__, __LINE__);
				return false;
			}
			var = ds[KEY_PARAM_STR];
			DynamicStruct param;
			try
			{
				param = var.extract<DynamicStruct>();
			}
			catch(Exception& e)
			{
				warnf("%s, %d: Extract param error[%s].", __FILE__, __LINE__, e.message().c_str());
				return false;
			}
			if(!param.contains(REG_UUID_STR) || !param.contains(REG_KEY_STR) || !param.contains(REG_TIMESTAMP_STR) || !param.contains(REG_TOKEN_STR))
			{
				warnf("%s, %d: Param miss uuid, key, timestamp, or token.", __FILE__, __LINE__);
				return false;
			}
			std::string uuid = param[REG_UUID_STR].toString();
			if(uuid != m_uuid)
			{
				warnf("%s, %d: Server response uuid error.", __FILE__, __LINE__);
				return false;
			}
			std::string recvkey = param[REG_KEY_STR].toString();
			std::string timestamp = param[REG_TIMESTAMP_STR].toString();
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
			m_token = param[REG_TOKEN_STR].toString();
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
	memset(buf, 0, sizeof(buf));
	if(m_sock->poll(Timespan(10, 0), Socket::SELECT_READ) > 0)
	{
		try
		{
			if(m_sock->receiveBytes(buf, sizeof(buf)) > 0)
			{
				tracef("%s, %d: Receive register response: %s.", __FILE__, __LINE__, buf);
				JSON::Parser parser;
				Dynamic::Var var = parser.parse(buf);
				JSON::Object::Ptr object = var.extract<JSON::Object::Ptr>();
				DynamicStruct ds = *object;
				if(ds.contains("result") && ds["result"].toString() == "good")
				{
					infof("%s, %d: Register successfully.", __FILE__, __LINE__);
					return true;
				}
				else
				{
					warnf("%s, %d: Register failed.", __FILE__, __LINE__);
					return false;
				}
			}
		}
		catch(Exception& e)
		{
			errorf("%s, %d: Receive exception[%s].", __FILE__, __LINE__, e.message().c_str());
		}
	}
	else
	{
		errorf("%s, %d: Receive register response timeout.", __FILE__, __LINE__);
	}
	dealError(PLAIN_SOCKET);
	return false;
}

void CRegProxy::createPacket(char* buf, UInt16 size, REQUEST_ACTION ra)
{
	DynamicStruct ds;
	ds[KEY_TYPE_STR] = TYPE_REQUEST_STR;
	switch(ra)
	{
		case ACTION_GETTOKEN :
		{
			DynamicStruct param;
			ds[KEY_ACTION_STR] = "server.token";
			param[REG_DEV_NAME_STR] = m_dev_name;
			param[REG_DEV_TYPE_STR] = m_dev_type;
			param[REG_DEV_MANU_STR] = m_manufacture;
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
	std::string bufStr = ds.toString();
	snprintf(buf, size - 1, "%s", bufStr.c_str());
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
			}
			else
			{
				Thread::sleep(5 * 1000);
			}
		}
		else
		{
			//receive connection request
			if(m_sock->poll(Timespan(5, 0), Socket::SELECT_READ|Socket::SELECT_ERROR))
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
					dealError(PLAIN_SOCKET);
					continue;
				}
				std::string request(buf);
				tracef("%s, %d: Receive from server:%s.", __FILE__, __LINE__, buf);
				JSON::Object::Ptr nil = NULL;
				m_rpc->addRequest(new RequestNotification((UInt64)m_sock, request, nil));
				Timestamp t;
				m_lastCheckTime = t;
			}
			else
			{
				Timestamp t;
				if(t - m_lastCheckTime < m_checkPeriod)
				{
					continue;
				}
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
	if(m_sock->sendBytes(buf, sizeof(buf)) > 0 )
	{
		tracef("%s, %d: Send keepalive buf:%s.", __FILE__, __LINE__, buf);
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

