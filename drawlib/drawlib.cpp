#include "manager.h"
#include "drawlib.h"

DRAWLIB_DECL int __stdcall Drawlib_Init()
{
    return CManager::GetInstance()->Init();
}

DRAWLIB_DECL int __stdcall Drawlib_GetMonitorWH(int iMonitorIndex, int *Width, int *Height)
{
    return CManager::GetInstance()->GetMonitorWH(iMonitorIndex,*Width, *Height);
}

DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex,bool bFullScreen)
{
    return CManager::GetInstance()->CreateVideoWindows(parent, pos, pBGFile, iMonitorIndex, bFullScreen);
}


DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindowsWithFull(const char* pBGFile, int iMonitorIndex)
{
    SRect pos;
    pos.x = pos.y = pos.width = pos.height = 0;
    return CManager::GetInstance()->CreateVideoWindows(pos,pBGFile, iMonitorIndex);
}

DRAWLIB_DECL int __stdcall Drawlib_CreateFullScreen(int iWinID)
{
    return CManager::GetInstance()->CreateFullScreen(iWinID);
}

DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowSize(int iWinID,int iWidth,int iHeight)
{
    return CManager::GetInstance()->SetVideoWindowSize(iWinID,iWidth,iHeight);
}

DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowPos(int iWinID,int x,int y)
{
    return CManager::GetInstance()->SetVideoWindowPos(iWinID,x,y);
}

DRAWLIB_DECL int __stdcall Drawlib_ResetSubScreenPos(int iWinID,int SubScreenID,SRect Pos)
{
    LOG_DEBUG << "Enter the function ResetSubScreenPos";
    LOG_DEBUG << "WinID:" << iWinID << ",SubScreenID:" << SubScreenID<<" x:"<<Pos.x<<" y:"<<Pos.y<<" w:"<<Pos.width<<" h:"<<Pos.height;
    bool bNeedToRestart = false;
    if (CManager::GetInstance()->isStartVideo(iWinID,SubScreenID))
    { 
        CManager::GetInstance()->StopPlay(iWinID, SubScreenID);
        bNeedToRestart = true;
    }
    
    if (-1 == CManager::GetInstance()->ChangeSubscreenPos(iWinID, SubScreenID, Pos))
    {
        return -1;
    }

    if (bNeedToRestart)
    {
        CManager::GetInstance()->StartPlay(iWinID, SubScreenID);
    }
    
    LOG_DEBUG << "Exit the function ResetSubScreenPos";
    return 0;
}

DRAWLIB_DECL int __stdcall Drawlib_CreateSubScreen(int iWinID,SRect pos)
{
    return CManager::GetInstance()->CreateSubScreen(iWinID,pos);
}


DRAWLIB_DECL int __stdcall Drawlib_DeleteSubScreen(int iWinID,int iSubScreenID)
{
    return CManager::GetInstance()->DeleteSubScreen(iWinID,iSubScreenID);
}

DRAWLIB_DECL int __stdcall Drawlib_UpdateImageData(int iWinID, 
                                        int SubScreenID, 
                                        unsigned char *pData, 
                                        int iWidth,
                                        int iHeight, 
                                        int iSize)
{
    return CManager::GetInstance()->UpdateImageData(iWinID,SubScreenID,pData,iWidth,iHeight,iSize);
}

DRAWLIB_DECL void __stdcall Drawlib_Uninit()
{
    CManager::GetInstance()->Release();
}

DRAWLIB_DECL int __stdcall Drawlib_GetMonitorCnt()
{
    return CManager::GetInstance()->GetMonitorCnt();
}

DRAWLIB_DECL int __stdcall Drawlib_SetSubSrceenVideoText(int iWinID,int iSubScreenID,SVideoTextInfo *pTextInfo)
{
    return CManager::GetInstance()->SetSubScreenVideoText(iWinID,iSubScreenID,pTextInfo);
}

DRAWLIB_DECL int __stdcall Drawlib_DisableSubSrceenVideoText(int iWinID,int iSubScreenID)
{
    return CManager::GetInstance()->DisableSubScreenVideoText(iWinID,iSubScreenID);
}

DRAWLIB_DECL void __stdcall Drawlib_SetMouseBtnCallBack(int iWinID,MouseButtonCallBack mouseBtnCallBack)
{
    CManager::GetInstance()->SetMousBtnActionCallback(iWinID,mouseBtnCallBack);
}

DRAWLIB_DECL void __stdcall Drawlib_SetCursorPosCallBack(int iWinID,CursorPosCallBack cursorPosCallBack)
{
    CManager::GetInstance()->SetCursorCallback(iWinID,cursorPosCallBack);
}

DRAWLIB_DECL int __stdcall Drawlib_GetSelectSubScreen(int iWinID)
{
    return CManager::GetInstance()->GetSelectSubScreen(iWinID);
}

DRAWLIB_DECL void __stdcall Drawlib_SelectSubscreen(int iWinID,int iSubScreenID)
{
    CManager::GetInstance()->SelectSubScreen(iWinID,iSubScreenID);
}

DRAWLIB_DECL void __stdcall Drawlib_SetFullScreen(int iWinID,int iSubScreenID)
{
    CManager::GetInstance()->SetFullScreen(iWinID,iSubScreenID);
}

DRAWLIB_DECL void __stdcall Drawlib_CancelFullScreen(int iWinID)
{
    CManager::GetInstance()->CancelFullScreen(iWinID);
}

DRAWLIB_DECL void __stdcall Drawlib_SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate)
{
    CManager::GetInstance()->SetTextMoveInfo(iWinID,SubScreenID,enPolicy,fRate);
}

DRAWLIB_DECL void __stdcall Drawlib_StartPlay(int iWinID,int SubScreenID)
{
    CManager::GetInstance()->StartPlay(iWinID,SubScreenID);
}

DRAWLIB_DECL void __stdcall Drawlib_StopPlay(int iWinID,int SubScreenID)
{
    CManager::GetInstance()->StopPlay(iWinID,SubScreenID);
}
