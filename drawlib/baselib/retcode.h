//因为在VC环境中已经定义了宏_RETCODE_H,所以在此修改这个宏
#ifndef _NETTVRETCODE_H
#define _NETTVRETCODE_H

enum ENRetCode
{
    Ret_Success = 0,
    Ret_Failed,
    
    //内存相关错误码
    Ret_AllocateMemFailed,

    //会话相关错误
    Ret_InvalidSessionID,
    Ret_SessionIDNotAllocated,
    Ret_SessionNotExist,

    //数据库相关错误码
    Ret_DBStatusInvlid, 
    Ret_ConnectDBFailed,
    Ret_ExecuteSQLFailed,
    Ret_GetRecordSetFailed,
    Ret_ConvertRecordSetFailed,
    Ret_InvalidSQL,
    Ret_InitDBFailed,

    //Socket相关错误码
    Ret_OpenServerFailed,
    Ret_ConnectServerFailed,

    //XML相关错误码
    Ret_LoadXMLFailed,

    //参数相关错误
    Ret_ParameterInvalid
};

#endif

