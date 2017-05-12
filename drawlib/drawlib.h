#ifndef DRAW_LIB_H
#define DRAW_LIB_H
#include "typedefine.h"

#ifdef WIN32
#ifndef USE_STATIC
    #ifdef  DRAWLIB_DLL
    #define DRAWLIB_DECL extern "C" __declspec(dllexport)
    #else
    #define DRAWLIB_DECL extern "C" __declspec(dllimport)
    #endif
#else
    #define DRAWLIB_DECL
    #define __stdcall
#endif
#else
    #define DRAWLIB_DECL
    #define __stdcall
#endif

//��ʼ�����ſ�
DRAWLIB_DECL int __stdcall Drawlib_Init();

//����ʼ�����ſ�
DRAWLIB_DECL void __stdcall Drawlib_Uninit();

//��ȡ��ǰ�ɹ��������ʾ������,��ʾ����index��0��ʼ,���Ϊ��ʾ��������һ
DRAWLIB_DECL int __stdcall Drawlib_GetMonitorCnt();

//����ָ����ʾ����index��ȡ��ʾ�������ֱ���
DRAWLIB_DECL int __stdcall Drawlib_GetMonitorWH(int iMonitorIndex,int *Width, int *Height);

/*����һ������
parent ���ڵĸ�����
pos ���ڵ�λ��
pBGFile ����ͼ�ļ�
iMonitorIndex ��ʾ��������ID
bFullScreen �����Ƿ�ȫ��
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex,bool bFullScreen = false);

/*����һ��ȫ������
pBGFile ����ͼ�ļ�
iMonitorIndex ��ʾ��������ID
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindowsWithFull(const char* pBGFile, int iMonitorIndex);

/*����һ��ȫ���ӷ���
iWinID Ϊ����ID
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateFullScreen(int iWinID);

/*���ô��ڴ�С
iWidth ��
iHeight ��
*/
DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowSize(int iWinID,int iWidth,int iHeight);

/*���ô���λ��
x����
y����
*/
DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowPos(int iWinID,int x,int y);

/*�����ӷ�����λ��
iWinID  ����ID
SubScreenID ����ID
pos �µ�λ����Ϣ
*/
DRAWLIB_DECL int __stdcall Drawlib_ResetSubScreenPos(int iWinID,int SubScreenID,SRect Pos);

/*����һ���ӷ���
*iWinIDΪCreateVideoWindows���صĴ���ID
*posΪ�����ڵ�λ��
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateSubScreen(int iWinID,SRect pos);

/*ɾ��һ���ӷ���
*iWinIDΪCreateVideoWindows���صĴ���ID
*iSubScreenIDΪ��Ӧ���ӷ���ID
*/
DRAWLIB_DECL int __stdcall Drawlib_DeleteSubScreen(int iWinID,int iSubScreenID);

/*����ָ��������ͼ��,����ͼ�񲥷�
*iWinIDΪCreateVideoWindows���صĴ���ID
*SubScreenIDΪ��Ӧ���ӷ���ID
*pDataΪͼ������,Ϊrgb��ʽ
*iWidthΪͼ��Ŀ�
*iHeightΪͼ��ĸ�
*iSizeΪͼ��Ĵ�С
*/
DRAWLIB_DECL int __stdcall Drawlib_UpdateImageData(int iWinID,
                                                int SubScreenID,
                                                unsigned char *pData,
                                                int iWidth,
                                                int iHeight,
                                                int iSize);

/*��ָ��������ָ��λ����ʾ����
*iWinID����ID
*iSubScreenIDΪ�ӷ���ID
*pTextInfo������Ϣ
*/
DRAWLIB_DECL int __stdcall Drawlib_SetSubSrceenVideoText(int iWinID,int iSubScreenID,SVideoTextInfo *pTextInfo);

/*ȡ����ʾ����*/
DRAWLIB_DECL int __stdcall Drawlib_DisableSubSrceenVideoText(int iWinID,int iSubScreenID);

/*���û�ȡ��갴���¼��Ļص�*/
DRAWLIB_DECL void __stdcall Drawlib_SetMouseBtnCallBack(int iWinID,MouseButtonCallBack mouseBtnCallBack);

/*���û�ȡ���λ�õĻص�*/
DRAWLIB_DECL void __stdcall Drawlib_SetCursorPosCallBack(int iWinID,CursorPosCallBack cursorPosCallBack);

DRAWLIB_DECL int __stdcall Drawlib_GetSelectSubScreen(int iWinID);

/*����ѡ�е��ӷ���*/
DRAWLIB_DECL void __stdcall Drawlib_SelectSubscreen(int iWinID,int iSubScreenID);

/*��ָ���ӷ�������Ϊȫ����ʾ*/
DRAWLIB_DECL void __stdcall Drawlib_SetFullScreen(int iWinID,int iSubScreenID);

/*ȡ���ӷ�����ȫ����ʾ*/
DRAWLIB_DECL void __stdcall Drawlib_CancelFullScreen(int iWinID);

/*�����Ӵ����������ƶ�����Ϣ*/
DRAWLIB_DECL void __stdcall Drawlib_SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate);

/*��ʼ����ͼ��*/
DRAWLIB_DECL void __stdcall Drawlib_StartPlay(int iWinID,int SubScreenID);

/*��������ͼ��*/
DRAWLIB_DECL void __stdcall Drawlib_StopPlay(int iWinID,int SubScreenID);

#endif