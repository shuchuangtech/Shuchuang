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
#define USER_USERNAME_STR "username"
#define USER_PASSWORD_STR "password"
#define USER_TOKEN_STR "token"
#define USER_CHALLENGE_STR "challenge"
#define USER_NEW_PASS_STR "newpassword"
//register
#define REG_TOKEN_STR "token"
#define REG_UUID_STR "uuid"
#define REG_DEV_NAME_STR "name"
#define REG_DEV_TYPE_STR "type"
#define REG_DEV_MANU_STR "manufacture"
#define REG_STATE_STR "state"
#define REG_TIMESTAMP_STR "timestamp"
#define REG_KEY_STR "key"
//task
//record
#define RECORD_STARTTIME_STR "starttime"
#define RECORD_ENDTIME_STR "endtime"
#define RECORD_RECORDS_STR "records"
//Component>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define COMPONENT_USER_STR "user"
#define COMPONENT_TASK_STR "task"
#define COMPONENT_DEVICE_STR "device"
#define COMPONENT_RECORD_STR "record"
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
#define TASK_METHOD_LIST "list"
//device
#define DEVICE_METHOD_OPEN "open"
#define DEVICE_METHOD_CLOSE "close"
#define DEVICE_METHOD_CHECK "check"
//record
#define RECORD_METHOD_GET "get"
//server
#define SERVER_METHOD_CHECK "check"
#define SERVER_METHOD_TOKEN "token"
#define SERVER_METHOD_REGISTER "register"
#define SERVER_METHOD_KEEPALIVE "keepalive"
//result>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define RESULT_GOOD_STR "good"
#define RESULT_FAIL_STR "failed"
#endif

