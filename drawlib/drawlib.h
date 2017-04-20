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

DRAWLIB_DECL int __stdcall Drawlib_Init();

DRAWLIB_DECL void __stdcall Drawlib_Uninit();

DRAWLIB_DECL int __stdcall Drawlib_GetMonitorCnt();

DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex,bool bFullScreen = false);

DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindowsWithFull(const char* pBGFile, int iMonitorIndex);

DRAWLIB_DECL int __stdcall Drawlib_CreateFullScreen(int iWinID);

DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowSize(int iWinID,int iWidth,int iHeight);

DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowPos(int iWinID,int x,int y);

DRAWLIB_DECL int __stdcall Drawlib_ResetSubScreenPos(int iWinID,int SubScreenID,SRect Pos);

DRAWLIB_DECL int __stdcall Drawlib_CreateSubScreen(int iWinID,SRect pos);

DRAWLIB_DECL int __stdcall Drawlib_DeleteSubScreen(int iWinID,int iSubScreenID);

DRAWLIB_DECL int __stdcall Drawlib_UpdateImageData(int iWinID,
                                                int SubScreenID,
                                                unsigned char *pData,
                                                int iWidth,
                                                int iHeight,
                                                int iSize);

DRAWLIB_DECL int __stdcall Drawlib_SetSubSrceenVideoText(int iWinID,int iSubScreenID,SVideoTextInfo *pTextInfo);

DRAWLIB_DECL int __stdcall Drawlib_DisableSubSrceenVideoText(int iWinID,int iSubScreenID);

DRAWLIB_DECL void __stdcall Drawlib_SetMouseBtnCallBack(int iWinID,MouseButtonCallBack mouseBtnCallBack);

DRAWLIB_DECL void __stdcall Drawlib_SetCursorPosCallBack(int iWinID,CursorPosCallBack cursorPosCallBack);

DRAWLIB_DECL int __stdcall Drawlib_GetSelectSubScreen(int iWinID);

DRAWLIB_DECL void __stdcall Drawlib_SelectSubscreen(int iWinID,int iSubScreenID);

DRAWLIB_DECL void __stdcall Drawlib_SetFullScreen(int iWinID,int iSubScreenID);

DRAWLIB_DECL void __stdcall Drawlib_CancelFullScreen(int iWinID);

DRAWLIB_DECL void __stdcall Drawlib_SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate);

DRAWLIB_DECL void __stdcall Drawlib_StartPlay(int iWinID,int SubScreenID);

DRAWLIB_DECL void __stdcall Drawlib_StopPlay(int iWinID,int SubScreenID);

#endif