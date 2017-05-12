#ifndef MANAGER_H
#define MANAGER_H

#include <map>
#include "draw.h"
#include "typedefine.h"
#include "log.h"

struct SMonitorInfo
{
    int iSeqNo;
    GLFWmonitor *pMonior;
    int iMaxWidth;
    int iMaxHeight;
};

#define MAX_SUBSRCEEN_CNT 16
class CManager
{
    public:
        static CManager* GetInstance();
        static void Release();

        int Init();
    public:
        
        int GetMonitorCnt();
        int GetMonitorWH(int iMonitorIndex,int &iWidth, int &iHeight);
        //����һ����Ƶ����
        int CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex, bool bFullScreen);
        int CreateVideoWindows(SRect pos,const char* pBGFile, int iMonitorIndex);
        int SetVideoWindowSize(int iWinID,int iWidth,int iHeight);
        int SetVideoWindowPos(int iWindID,int x,int y);

        //�����Ӵ��ڵĴ�С
        int ChangeSubscreenPos(int iWindID,int iSubScreenID,SRect Pos);

        //����һ��ȫ�����Ӵ���
        int CreateFullScreen(int iWindID);
        //����һ���Ӵ���
        int CreateSubScreen(int winID,SRect pos);
        int DeleteSubScreen(int iWinID,int iSubScreenID);


        int UpdateImageData(int iWinID,
                            int iScreenID,
                            unsigned char* pData,
                            unsigned int iWidth,
                            unsigned int iHeight,
                            unsigned int iSize);
        //��Ƶ��Ļ���
        int SetSubScreenVideoText(int winID,int iScreenID,SVideoTextInfo *pTextInfo);
        int DisableSubScreenVideoText(int winID,int iScreenID);

        void PrintFrameRate(int winID,int iScreenID,bool bEnable);

        //��������¼��Ļص�
        void SetMousBtnActionCallback(int iWinID,MouseButtonCallBack f);
        void SetCursorCallback(int iWinID,CursorPosCallBack f);

        int GetSelectSubScreen(int iWinID);

        void SelectSubScreen(int iWinID,int iSubScreen);

        void SetFullScreen(int iWinID,int iSubScreenID);
        void CancelFullScreen(int iWinID);

        void SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate);

        bool isStartVideo(int iWinID, int iSubScreenID);
        void StartPlay(int iWinID,int iSubScreenID);
        void StopPlay(int iWinID, int iSubScreenID);
        //��ʼ��Ⱦ���еĴ�����Ƶ
        int Start();
        int StartWithFull();

    private:
        static int WindThread(void *arg);
        static int WindThreadForFull(void *arg);
        static int DrawThread(void *arg);

    private:
        CManager();
        ~CManager();

    private:
        static CManager *m_pInstance;
    
    private:

        struct SWindowData
        {
            SWindowData()
            {
                pos.height = 0;
                pos.width = 0;
                pos.x = 0;
                pos.y = 0;

                pDraw = NULL;
#ifdef WIN32
                pParent = NULL;
#endif
                bNeedToClose = false;
                bFullScreen = false;

                mtx_init(&m_mutSubScreen,mtx_plain);
                cnd_init(&m_cndSubScreen);

                mtx_init(&m_mutCreateScreen,mtx_plain);
            }

            ~SWindowData()
            {
                mtx_destroy(&m_mutSubScreen);
                cnd_destroy(&m_cndSubScreen);

                mtx_destroy(&m_mutCreateScreen);
            }

            struct SSubScreenData
            {
                SSubScreenData()
                {
                    mtx_init(&mtxCreateScreen,mtx_plain);
                    cnd_init(&cndCreateScreen);

                    iSubSrceenID = 0;
                }

                int iSubSrceenID;
                SRect rect;
                mtx_t mtxCreateScreen;
                cnd_t cndCreateScreen;
            };

            typedef boost::shared_ptr<SSubScreenData> SSubScreenPosPtr;

            SRect pos;
            CDraw *pDraw;
#ifdef WIN32
            HWND *pParent;
#endif
            bool bNeedToClose;
            thrd_t threadID;

            std::deque<SSubScreenPosPtr> deqSubSrceenPos;

            cnd_t m_cndSubScreen;
            mtx_t m_mutSubScreen;

            //�����Ӵ��ڵĻ�����
            mtx_t m_mutCreateScreen;

            bool bFullScreen;
        };

        std::map<int,SWindowData*> m_mapVideoWindows;

    private:
        thrd_t m_ManagerThrdID;

        int m_iWndID;
        bool m_bRunning;

        //�������ڵ�notify
        cnd_t m_cndCreateWindow;
        mtx_t m_mutCreateWindow;

        //������Ⱦ�̵߳�notify
        cnd_t m_cndCreateDraw;
        mtx_t m_mutCreateDraw;

        int m_iGenerSubScreenID;

        /*unsigned int *m_pSubScreenID;*/
        //���ڷ���subscreen id
        std::vector<unsigned int> m_vecSubScreenID;
        
        std::vector<SMonitorInfo> m_vecMonitorInfos;
        int m_iCurrUseMonitorSeq;
};

#endif