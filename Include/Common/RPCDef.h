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
#define KEY_TOKEN_STR "token"
#define KEY_PARAM_STR "param"
#define KEY_REQUEST_ID_STR "requestid"
#define KEY_RESULT_STR "result"
#define KEY_DETAIL_STR "detail"
//type×Ö¶Î
#define TYPE_REQUEST_STR "request"
#define TYPE_RESPONSE_STR "response"
//action×Ö¶Î component.method
//param>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//user
#define PARAM_USERNAME_STR "username"
#define PARAM_PASSWORD_STR "password"
#define PARAM_TOKEN_STR "token"
#define PARAM_CHALLENGE_STR "challenge"
//register
#define PARAM_UUID_STR "uuid"
#define PARAM_DEV_NAME_STR "name"
#define PARAM_DEV_TYPE_STR "type"
#define PARAM_DEV_MANU_STR "manufacture"
#define PARAM_STATE_STR "state"
#define PARAM_TIMESTAMP_STR "timestamp"
#define PARAM_KEY_STR "key"
//Component
#define COMPONENT_USER_STR "user"
#define COMPONENT_TASK_STR "task"
#define COMPONENT_DEVICE_STR "device"
#define COMPONENT_SERVER_STR "server"
//Method>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//user
#define USER_METHOD_LOGIN "login"
#define USER_METHOD_LOGOUT "logout"
#define USER_METHOD_PASSWD "passwd"
//task
#define TASK_METHOD_ADD "add"
#define TASK_METHOD_REMOVE "remove"
#define TASK_METHOD_MODIFY "modify"
#define TASK_METHOD_GETTASKS "gettasks"
//device
#define DEVICE_METHOD_OPEN "open"
#define DEVICE_METHOD_CLOSE "close"
#define DEVICE_METHOD_CHECK "check"
//server
#define SERVER_METHOD_CHECK "check"
#define SERVER_METHOD_TOKEN "token"
#define SERVER_METHOD_REGISTER "register"
#define SERVER_METHOD_KEEPALIVE "keepalive"
//result>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define RESULT_GOOD_STR "good"
#define RESULT_FAIL_STR "failed"
#endif

