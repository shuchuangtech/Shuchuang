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
#define USER_BINDUSER_STR "binduser"
#define USER_PASSWORD_STR "password"
#define USER_TOKEN_STR "token"
#define USER_CHALLENGE_STR "challenge"
#define USER_NEW_PASS_STR "newpassword"
#define USER_AUTHORITY_STR "authority"
#define USER_REMAINOPEN_STR "remainopen"
#define USER_TIMEOFVALIDITY_STR "timeofvalidity"
#define USER_LIMIT_STR "limit"
#define USER_OFFSET_STR "offset"
//register
#define REG_TOKEN_STR "token"
#define REG_UUID_STR "uuid"
#define REG_DEV_NAME_STR "name"
#define REG_DEV_TYPE_STR "type"
#define REG_DEV_MANU_STR "manufacture"
#define REG_STATE_STR "state"
#define REG_TIMESTAMP_STR "timestamp"
#define REG_KEY_STR "key"
#define REG_MOBILETOKEN_STR "mobiletoken"
//task
#define TASK_ID_STR "id"
#define TASK_OPTION_STR "option"
#define TASK_HOUR_STR "hour"
#define TASK_MINUTE_STR "minute"
#define TASK_WEEKDAY_STR "weekday"
#define TASK_ACTIVE_STR "active"
//record
#define RECORD_STARTTIME_STR "starttime"
#define RECORD_ENDTIME_STR "endtime"
#define RECORD_LIMIT_STR "limit"
#define RECORD_OFFSET_STR "offset"
#define RECORD_RECORDS_STR "records"
//device
#define DEVICE_STATE_STR "state"
#define DEVICE_SWITCH_STR "switch"
#define DEVICE_OPEN_STR "open"
#define DEVICE_CLOSE_STR "close"
#define DEVICE_MODE_STR "mode"
//system
#define SYSTEM_TYPE_STR "type"
#define SYSTEM_VERSION_STR "version"
#define SYSTEM_BUILDTIME_STR "buildtime"
#define SYSTEM_CHECKSUM_STR "checksum"
//update
#define UPDATE_TYPE_STR "type"
#define UPDATE_VERSION_STR "version"
#define UPDATE_BUILDTIME_STR "buildtime"
#define UPDATE_CHECKSUM_STR "checksum"
#define UPDATE_NEWFEATURE_STR "newfeature"
//Component>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define COMPONENT_USER_STR "user"
#define COMPONENT_TASK_STR "task"
#define COMPONENT_DEVICE_STR "device"
#define COMPONENT_RECORD_STR "record"
#define COMPONENT_SERVER_STR "server"
#define COMPONENT_SYSTEM_STR "system"
#define COMPONENT_UPDATE_STR "update"
//Method>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//user
#define USER_METHOD_LOGIN "login"
#define USER_METHOD_LOGOUT "logout"
#define USER_METHOD_PASSWD "passwd"
#define USER_METHOD_ADD "add"
#define USER_METHOD_DELETE "delete"
#define USER_METHOD_TOPUP "topup"
#define USER_METHOD_LIST "list"
//task
#define TASK_METHOD_ADD "add"
#define TASK_METHOD_REMOVE "remove"
#define TASK_METHOD_MODIFY "modify"
#define TASK_METHOD_LIST "list"
//device
#define DEVICE_METHOD_OPEN "open"
#define DEVICE_METHOD_CLOSE "close"
#define DEVICE_METHOD_CHECK "check"
#define DEVICE_METHOD_RESET "reset"
#define DEVICE_METHOD_RESTART "restart"
#define DEVICE_METHOD_CHMODE "chmode"
#define DEVICE_METHOD_GETMODE "getmode"
//record
#define RECORD_METHOD_GET "get"
//server
#define SERVER_METHOD_CHECK "check"
#define SERVER_METHOD_TOKEN "token"
#define SERVER_METHOD_REGISTER "register"
#define SERVER_METHOD_KEEPALIVE "keepalive"
#define SERVER_METHOD_BIND "bind"
#define SERVER_METHOD_UNBIND "unbind"
//system
#define SYSTEM_METHOD_RESET "reset"
#define SYSTEM_METHOD_UPDATE "update"
#define SYSTEM_METHOD_VERSION "version"
//update
#define UPDATE_METHOD_CHECK "check"
//result>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define RESULT_GOOD_STR "good"
#define RESULT_FAIL_STR "failed"
#endif

