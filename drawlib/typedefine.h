#ifndef TYPE_DEFINE_H
#define TYPE_DEFINE_H

#define STR_LEN 128

#ifdef WIN32
typedef wchar_t WCHar;
#else
typedef char WCHar;
#endif

#include <memory>
#include <string.h>
struct SRect
{
    int x;
    int y;
    int width;
    int height;
};

//字幕相关
enum ENTextSize
{
    ENTextSize_Big = 0,
    ENTextSize_Normal,
    ENTextSize_Small
};

struct STextColor
{
    int R;
    int G;
    int B;
};

struct SVideoTextInfo
{
    SVideoTextInfo()
    {
        x = -1;
        y = -1;
        color.R = 255;
        color.B = 0;
        color.G = 0;
        memset(szText,0,STR_LEN);
    }

    int x;
    int y;
    STextColor color;
    ENTextSize textSize;
    WCHar szText[STR_LEN];

};

//事件相关
enum ENMouseBtn
{
    //左键
    ENMouseBtn_LEFT = 0,
    //右键
    ENMouseBtn_RIGHT,
    //中间键
    ENMouseBtn_MIDDLE
};

enum ENMouseBtnAction
{
    //鼠标键释放
    ENMouseAction_RELEASE = 0,
    //鼠标键按下
    ENMouseAction_PRESS
};

enum enMovePolicy
{
    enMovePolicy_NotMove = -1,
    enMovePolicy_Horizontal = 0,
    enMovePolicy_vertical
};

typedef void (*CursorPosCallBack)(double,double);
typedef void (*MouseButtonCallBack)(ENMouseBtn enBtn,ENMouseBtnAction enAction);

#endif
