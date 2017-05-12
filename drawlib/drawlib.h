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

//初始化播放库
DRAWLIB_DECL int __stdcall Drawlib_Init();

//反初始化播放库
DRAWLIB_DECL void __stdcall Drawlib_Uninit();

//获取当前可供输出的显示器数量,显示器的index从0开始,最大为显示器数量减一
DRAWLIB_DECL int __stdcall Drawlib_GetMonitorCnt();

//根据指定显示器的index获取显示器的最大分辨率
DRAWLIB_DECL int __stdcall Drawlib_GetMonitorWH(int iMonitorIndex,int *Width, int *Height);

/*创建一个窗口
parent 窗口的父窗口
pos 窗口的位置
pBGFile 背景图文件
iMonitorIndex 显示器的索引ID
bFullScreen 窗口是否全屏
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex,bool bFullScreen = false);

/*创建一个全屏窗口
pBGFile 背景图文件
iMonitorIndex 显示器的索引ID
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateVideoWindowsWithFull(const char* pBGFile, int iMonitorIndex);

/*创建一个全屏子分屏
iWinID 为窗口ID
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateFullScreen(int iWinID);

/*设置窗口大小
iWidth 宽
iHeight 高
*/
DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowSize(int iWinID,int iWidth,int iHeight);

/*设置窗口位置
x坐标
y坐标
*/
DRAWLIB_DECL int __stdcall Drawlib_SetVideoWindowPos(int iWinID,int x,int y);

/*重设子分屏的位置
iWinID  窗口ID
SubScreenID 分屏ID
pos 新的位置信息
*/
DRAWLIB_DECL int __stdcall Drawlib_ResetSubScreenPos(int iWinID,int SubScreenID,SRect Pos);

/*创建一个子分屏
*iWinID为CreateVideoWindows返回的窗口ID
*pos为窗口内的位置
*/
DRAWLIB_DECL int __stdcall Drawlib_CreateSubScreen(int iWinID,SRect pos);

/*删除一个子分屏
*iWinID为CreateVideoWindows返回的窗口ID
*iSubScreenID为对应的子分屏ID
*/
DRAWLIB_DECL int __stdcall Drawlib_DeleteSubScreen(int iWinID,int iSubScreenID);

/*更新指定分屏的图像,用于图像播放
*iWinID为CreateVideoWindows返回的窗口ID
*SubScreenID为对应的子分屏ID
*pData为图像数据,为rgb格式
*iWidth为图像的宽
*iHeight为图像的高
*iSize为图像的大小
*/
DRAWLIB_DECL int __stdcall Drawlib_UpdateImageData(int iWinID,
                                                int SubScreenID,
                                                unsigned char *pData,
                                                int iWidth,
                                                int iHeight,
                                                int iSize);

/*在指定分屏的指定位置显示文字
*iWinID窗口ID
*iSubScreenID为子分屏ID
*pTextInfo文字信息
*/
DRAWLIB_DECL int __stdcall Drawlib_SetSubSrceenVideoText(int iWinID,int iSubScreenID,SVideoTextInfo *pTextInfo);

/*取消显示文字*/
DRAWLIB_DECL int __stdcall Drawlib_DisableSubSrceenVideoText(int iWinID,int iSubScreenID);

/*设置获取鼠标按键事件的回调*/
DRAWLIB_DECL void __stdcall Drawlib_SetMouseBtnCallBack(int iWinID,MouseButtonCallBack mouseBtnCallBack);

/*设置获取鼠标位置的回调*/
DRAWLIB_DECL void __stdcall Drawlib_SetCursorPosCallBack(int iWinID,CursorPosCallBack cursorPosCallBack);

DRAWLIB_DECL int __stdcall Drawlib_GetSelectSubScreen(int iWinID);

/*设置选中的子分屏*/
DRAWLIB_DECL void __stdcall Drawlib_SelectSubscreen(int iWinID,int iSubScreenID);

/*将指定子分屏设置为全屏显示*/
DRAWLIB_DECL void __stdcall Drawlib_SetFullScreen(int iWinID,int iSubScreenID);

/*取消子分屏的全屏显示*/
DRAWLIB_DECL void __stdcall Drawlib_CancelFullScreen(int iWinID);

/*设置子窗口上文字移动的信息*/
DRAWLIB_DECL void __stdcall Drawlib_SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate);

/*开始播放图像*/
DRAWLIB_DECL void __stdcall Drawlib_StartPlay(int iWinID,int SubScreenID);

/*结束播放图像*/
DRAWLIB_DECL void __stdcall Drawlib_StopPlay(int iWinID,int SubScreenID);

#endif