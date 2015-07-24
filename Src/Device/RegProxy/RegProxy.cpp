#include "Device/RegProxy.h"
#include "Common/PrintLog.h"
#include "Common/RPCDef.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Timespan.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Observer.h"
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

void CRegProxy::setSecureServerInfo(std::string ssl_host, UInt16 ssl_port)
{
	m_ssl_host = ssl_host;
	m_ssl_port = ssl_port;
}

void CRegProxy::setServerInfo(std::string reg_host, UInt16 reg_port)
{
	m_reg_host = reg_host;
	m_reg_port = reg_port;
}

void CRegProxy::handleNf(RequestNotification* pNf)
{
	RequestNotification::Ptr p(pNf);
	debugf("%s, %d: p->id:%d, this:%d\n", __FILE__, __LINE__, p->getID(), (int)this);
	if(p->getID() == (int)m_sock)
	{
		DynamicStruct ds = *(p->getParam());
		std::string param = ds.toString();
		debugf("%s, %d: Receive notification:%s.\n", __FILE__, __LINE__, param.c_str());
		if(m_sock == 0)
		{
			errorf("%s, %d: Tcp connection unestablished.\n", __FILE__, __LINE__);
			return;
		}
		tracef("%s, %d: Send request response\n", __FILE__, __LINE__);
		m_sock->sendBytes(param.c_str(), param.length());
	}
}

void CRegProxy::start()
{
	if(m_started)
	{
		warnf("%s, %d: RegProxy already started.\n", __FILE__, __LINE__);
		return;
	}
	m_rpc = CRPCServer::instance();
	if(m_rpc == NULL)
	{
		errorf("%s, %d: Get RPCServer instance failed.\n", __FILE__, __LINE__);
		return;
	}
	Observer<CRegProxy, RequestNotification> observer(*this, &CRegProxy::handleNf);
	m_rpc->addObserver(observer);	
	m_timer.setPeriodicInterval(0);
	m_timer.setStartInterval(500);
	m_started = true;
	TimerCallback<CRegProxy> callback(*this, &CRegProxy::onTimer);
	m_timer.start(callback);
	infof("%s, %d: RegProxy start succefully.\n", __FILE__, __LINE__);
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
	infof("%s, %d: RegProxy stopped.\n", __FILE__, __LINE__);
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
		tracef("%s, %d: Connecting to %s.\n", __FILE__, __LINE__, saddr.toString().c_str());
		Context::Ptr pContext = new Context(Context::TLSV1_CLIENT_USE, "", Context::VERIFY_NONE);
		m_ssl_sock = new SecureStreamSocket(pContext);
		m_ssl_sock->connect(saddr, Timespan(3, 0));
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Connect failed %s.\n", __FILE__, __LINE__, e.message().c_str());
		dealError(SECURE_SOCKET);
		return false;
	}
	tracef("%s, %d: Connect to the server successfully.\n", __FILE__, __LINE__);
	char buf[128] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_GETTOKEN);
	infof("%s, %d: Get token buf: %s.\n", __FILE__, __LINE__, buf);
	if(m_ssl_sock->sendBytes(buf, sizeof(buf)) > 0)
	{
		tracef("%s, %d: Send get token message successfully.\n", __FILE__, __LINE__);
	}
	else
	{
		dealError(SECURE_SOCKET);
		errorf("%s, %d: Send get token message failed.\n", __FILE__, __LINE__);
		return false;
	}
	memset(buf, 0 ,sizeof(buf));
	Timespan ts(10, 0);
	m_ssl_sock->setReceiveTimeout(ts);
	try
	{
		if(m_ssl_sock->receiveBytes(buf, (UInt16)sizeof(buf)) > 0)
		{
			JSON::Parser parser;
			Dynamic::Var var = parser.parse(buf);
			JSON::Object::Ptr obj = var.extract<JSON::Object::Ptr>();
			DynamicStruct ds = *obj;
			if(ds.contains("result") && ds.contains("token") && ds["result"].toString() == "good")
				m_token = ds["token"].toString();
			else
				return false;
			return true;
		}
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Receive token timeout[%s].\n", __FILE__, __LINE__, e.message().c_str());
		dealError(SECURE_SOCKET);
		return false;
	}
	return false;
}

bool CRegProxy::registerToServer()
{
	if(!getRegisterToken())
	{
		warnf("%s, %d: Get register token failed.\n", __FILE__, __LINE__);
		return false;
	}
	SocketAddress saddr(m_reg_host, m_reg_port);
	try
	{
		tracef("%s, %d: Connecting to %s.\n", __FILE__, __LINE__, saddr.toString().c_str());
		if(m_sock != 0)
		{
			m_sock->close();
			delete m_sock;
		}
		m_sock = new StreamSocket();
		m_sock->connect(saddr, Timespan(3, 0));
	}
	catch(Exception& e)
	{
		errorf("%s, %d: Connect failed[%s].\n", __FILE__, __LINE__, e.message().c_str());
		dealError(PLAIN_SOCKET);
		return false;
	}
	tracef("%s, %d: Connect to the server successfully.\n", __FILE__, __LINE__);
	char buf[128] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_REGISTER);
	infof("%s, %d: Register buf: %s.\n", __FILE__, __LINE__, buf);
	if(m_sock->sendBytes(buf, sizeof(buf)) > 0)
	{
		tracef("%s, %d: Register information sent.\n", __FILE__, __LINE__);
	}
	else
	{
		dealError(PLAIN_SOCKET);
		errorf("%s, %d: Send register message failed.\n", __FILE__, __LINE__);
		return false;
	}
	memset(buf, 0, sizeof(buf));
	Timespan ts(10, 0);
	if(m_sock->poll(ts, Socket::SELECT_READ) > 0)
	{
		try
		{
			if(m_sock->receiveBytes(buf, sizeof(buf)) > 0)
			{
				//tracef("%s, %d: Receive response: %s.\n", __FILE__, __LINE__, buf);
				//SocketAddress peer;
				//peer = m_sock->address();
				//tracef("%s, %d: peer address %s:%u.\n", __FILE__, __LINE__, peer.host().toString().c_str(), peer.port());
				JSON::Parser parser;
				Dynamic::Var var = parser.parse(buf);
				JSON::Object::Ptr object = var.extract<JSON::Object::Ptr>();
				DynamicStruct ds = *object;
				if(ds.contains("result") && ds["result"].toString() == "good")
				{
					tracef("%s, %d: Register successfully.\n", __FILE__, __LINE__);
					return true;
				}
				else
				{
					tracef("%s, %d: Register failed.\n", __FILE__, __LINE__);
					return false;
				}
			}
		}
		catch(Exception& e)
		{
			errorf("%s, %d: Receive exception[%s].\n", __FILE__, __LINE__, e.message().c_str());
		}
	}
	else
	{
		errorf("%s, %d: Receive register response timeout.\n", __FILE__, __LINE__);
	}
	dealError(PLAIN_SOCKET);
	return false;
}

void CRegProxy::createPacket(char* buf, UInt16 size, REQUEST_ACTION ra)
{
	DynamicStruct ds;
	switch(ra)
	{
		case ACTION_GETTOKEN :
			ds["action"] = "gettoken";
			ds["key"] = "alpha2015";
			break;
		case ACTION_REGISTER : 
			ds["action"] = "register";
			ds["token"] = m_token;
			break;
		case ACTION_KEEPALIVE :
			ds["action"] = "keepalive";
			break;
		default :
			break;
	}
	ds["uuid"] = "hjhjhjhjhj";
	ds["devType"] = "abc";
	std::string bufStr = ds.toString();
	snprintf(buf, size - 1, "%s", bufStr.c_str());
}

bool CRegProxy::parseAction(std::string& opt, std::string& component, std::string& method)
{
	std::string::size_type pos;
	pos = opt.find(".");
	if(pos == std::string::npos)
	{
		warnf("%s, %d: Action parse failed.\n", __FILE__, __LINE__);
		return false;
	}
	component = opt.substr(0, pos);
	method = opt.substr(pos + 1, opt.length() - pos - 1);
	return true;
}
/*
bool CRegProxy::handleRequest(DynamicStruct* param)
{
	if(!(*param).contains(KEY_ACTION_STR))
		return false;
	std::string opt = (*param)[KEY_ACTION_STR].toString();
	std::string component;
	std::string method;
	parseAction(opt, component, method);
	if(component != COMPONENT_CONN_STR)
		return false;
	if(method != CONN_METHOD_CONNECT)
		return false;
	return true;
}
*/
void CRegProxy::onTimer(Timer& timer)
{
	char buf[256] = {0, };
	Timespan ts(5, 0);
	while(m_started)
	{
		if(m_sock == 0)
		{
			warnf("%s, %d: Tcp disconnected.\n", __FILE__, __LINE__);
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
			if(m_sock->poll(ts, Socket::SELECT_READ|Socket::SELECT_ERROR))
			{
				if(!m_sock->available())
				{
					warnf("%s, %d: Socket not available.\n", __FILE__, __LINE__);
					dealError(PLAIN_SOCKET);
					continue;
				}
				memset(buf, 0, sizeof(buf));
				int ret = m_sock->receiveBytes(buf, sizeof(buf));
				if(ret <= 0 )
				{
					warnf("%s, %d: Receive error.\n", __FILE__, __LINE__);
					continue;
				}
				JSON::Parser parser;
				Dynamic::Var request;
				JSON::Object::Ptr object;
				DynamicStruct param;
				try
				{
					request = parser.parse(buf);
					object = request.extract<JSON::Object::Ptr>();
					m_rpc->addRequest(new RequestNotification((int)m_sock, object));
					//handleRequest(&param);
				}
				catch(Exception& e)
				{
					errorf("%s, %d: Request format error[%s].\n", __FILE__, __LINE__, e.message().c_str());
				}
				Timestamp t;
				m_lastCheckTime = t;
			}
			else
			{
				/*
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
						warnf("%s, %d: Send keepalive timeout.\n", __FILE__, __LINE__);
						dealError(SECURE_SOCKET|PLAIN_SOCKET);
					}
				}
				*/
			}		
		}
	}
}

bool CRegProxy::sendKeepAlive()
{
	char buf[128] = {0, };
	createPacket(buf, (UInt16)sizeof(buf), ACTION_KEEPALIVE);
	infof("%s, %d: KeepAlive buf: %s.\n", __FILE__, __LINE__, buf);
	if(m_sock->sendBytes(buf, sizeof(buf)) > 0 )
	{
		tracef("%s, %d: KeepAlive successfully.\n", __FILE__, __LINE__);
		return true;
	}
	else
	{
		warnf("%s, %d: KeepAlive sent failed.\n", __FILE__, __LINE__);
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

