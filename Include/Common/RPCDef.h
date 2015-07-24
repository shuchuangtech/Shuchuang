#ifndef __RPC_DEF_H__
#define __RPC_DEF_H__
/*
request
["type"] = "request"/"response"
["action"] = "component.method"
["param"]
["requestid"]
["result"]
*/
//×Ö¶ÎÃû
#define KEY_TYPE_STR "type"
#define KEY_ACTION_STR "action"
#define KEY_PARAM_STR "param"
#define KEY_REQUEST_ID_STR "requestid"
#define KEY_RESULT_STR "result"
#define KEY_DETAIL_STR "detail"
//type×Ö¶Î
#define TYPE_REQUEST_STR "request"
#define TYPE_RESPONSE_STR "response"
//action×Ö¶Î component.method
//param>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define PARAM_USERNAME_STR "username"
#define PARAM_PASSWORD_STR "password"
#define PARAM_TOKEN_STR "token"
#define PARAM_CHALLENGE_STR "challenge"
#define PARAM_SESSION_STR "session"
#define PARAM_HOST_STR "host"
#define PARAM_PORT_STR "port"
#define PARAM_UUID_STR "uuid"
#define PARAM_DEV_NAME_STR "dev_name"
#define PARAM_DEV_TYPE_STR "dev_type"
#define PARAM_STATE_STR "state"
//Component
#define COMPONENT_CONN_STR "conn"
#define COMPONENT_USER_STR "user"
#define COMPONENT_DEVICE_STR "device"
#define COMPONENT_SERVER_STR "server"
//Method>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//user
#define USER_METHOD_LOGIN "login"
#define USER_METHOD_LOGOUT "logout"
#define USER_METHOD_PASSWD "passwd"
//device
#define DEVICE_METHOD_OPEN "open"
#define DEVICE_METHOD_CLOSE "close"
//server
#define SERVER_METHOD_CHECK "check"
//result>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define RESULT_GOOD_STR "good"
#define RESULT_FAIL_STR "failed"
#endif

