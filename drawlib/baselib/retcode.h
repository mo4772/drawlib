//��Ϊ��VC�������Ѿ������˺�_RETCODE_H,�����ڴ��޸������
#ifndef _NETTVRETCODE_H
#define _NETTVRETCODE_H

enum ENRetCode
{
    Ret_Success = 0,
    Ret_Failed,
    
    //�ڴ���ش�����
    Ret_AllocateMemFailed,

    //�Ự��ش���
    Ret_InvalidSessionID,
    Ret_SessionIDNotAllocated,
    Ret_SessionNotExist,

    //���ݿ���ش�����
    Ret_DBStatusInvlid, 
    Ret_ConnectDBFailed,
    Ret_ExecuteSQLFailed,
    Ret_GetRecordSetFailed,
    Ret_ConvertRecordSetFailed,
    Ret_InvalidSQL,
    Ret_InitDBFailed,

    //Socket��ش�����
    Ret_OpenServerFailed,
    Ret_ConnectServerFailed,

    //XML��ش�����
    Ret_LoadXMLFailed,

    //������ش���
    Ret_ParameterInvalid
};

#endif

