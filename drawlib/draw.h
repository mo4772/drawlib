#ifndef DRAW_H
#define DRAW_H
#include <vector>
#include <map>
#include <deque>
#include <iostream>
#include "typedefine.h"
#include "gltool/gltools.h"
#include "glfw3.h"
#include "glfw3native.h"
#include "tinycthread.h"
#include "boost/smart_ptr.hpp"
#include "Timer.h"
#include "pcdeque.h"
#include "rendertextmanager.h"
#include "log.h"

#define GLEW_STATIC

/*#ifdef WIN32
typedef HWND Window;
#endif*/

enum ENDrawMode
{
    ENDrawMode_FULL = 0,
    ENDrawMode_WINDOWS
};
class CDraw
{
    
    public:
        CDraw();
        CDraw(GLFWmonitor *pMonitors);
        ~CDraw();


    public:
        int Init();
        //������Ƶ��ʾ����
        //����һ���ӷ���
        int UpdateSubScreenImage(unsigned int ID,
                                 unsigned char* pData,
                                 unsigned int iWidth,
                                 unsigned int iHeigth,
                                 unsigned int iSize);

        //��ȡ�ö����Ӧ�Ĵ��ھ��
        GLFWwindow *GetVideoSrceenHanlde();
        void StartDrawVideo(unsigned int iSubscrrenID);
        void StopDrawVideo(unsigned int iSubscrrenID);
        bool isStartVideo(unsigned int iSubSrceenID);
        int CreateVideoSrceen(SRect rect);
        int CreateVideoSrceen(void *parent,SRect rect,bool bFullScreen);
        int CreateSubScreen(SRect rect,int SubSrceenID);
        int DeleteSubScreen(int SubScreenID);
        int ChangeSubScreenPos(SRect rect, int SubSrceenID);
        int ModifySubScreenSize(int SubSrceenID,int iWidth,int iHeight);

        //��Ƶ��Ļ���
        int SetSubScreenVideoText(int SubsrceenID,SVideoTextInfo *pInfo);
        int DisableSubScreenVideoText(int SubsrceenID);

        //��ȡ������֡��
        int GetFrameRate(int SubsrceenID);
        //��ӡ֡��
        void PrintFrameRate(int SubsrceenID,bool bEnable);
        
        int PrepareDrawPic();

        int DrawPic();
        //���ô��ڴ�С
        int SetWindowSize(int iWidth,int iHeight);
        int SetWindowPos(int x,int y);

        //�����Ӵ��ڴ�С
        int ChangeSubscreenSize(int SubsrceenID,int iWidth,int iHeight);

        void SetMouseBtnCallback(MouseButtonCallBack f);

        void SetCursorCallback(CursorPosCallBack f);

        int GetSelectSubScreen();

        //�����Ƿ�ѡ��ĳһ����
        void SelectSubScreen(int iSubScreenID);

        void SetFullScreen(int iSubScreenID);
        void CancelFullScreen();

        bool ChangeToFullDraw();
        
        ENDrawMode GetDrawMode()
        {
            return m_enType;
        }

        void SetScreenWH(unsigned int uWidth,unsigned int uHeight)
        {
            m_uScreenWidth = uWidth;
            m_uScreenHeight = uHeight;
        }

        int SetTextMoveInfo(int SubScreenID,enMovePolicy enPolicy,float fRate);

    public:
        std::string m_strBG;
        GLuint m_uBGTextureID;

    private:
        static int FullScreenThread(void *arg);

    private:
        void UpdateData(unsigned char* pData,unsigned int iWidth,unsigned int Height,unsigned int iSize);
    
    private:
        static void ChangeSizeFun(GLFWwindow* pWindow,int iWidth,int iHeight);
        static void ErrorInfo(int errorno ,const char* errorinfo);

        static void MouseButFun(GLFWwindow* pWindow,int button,int action,int mods);
        static void CursorposFun(GLFWwindow* pWindow,double x,double y);
    
    private:
        GLShaderManager m_shaderManager;
        SRect m_VideoSrceenRect;

        typedef boost::shared_array<unsigned char> VideoDataPtr;
        struct SVideoData
        {
            SVideoData()
            {
                iWidth = 0;
                iHeight = 0;
                iSize = 0;
            }

            ~SVideoData()
            {
                iWidth = 0;
                iHeight = 0;
                iSize = 0;
            }

            unsigned int iWidth;
            unsigned int iHeight;
            unsigned int iSize;
            //unsigned char* pData;
            VideoDataPtr pData;
        };

        //��Ƶ�ӷ�����ص�����
        struct SSubSrceenData
        {
            SSubSrceenData()
            {
                ID = -1;
                pPic = NULL;
                memset(uPBOs,0,2);
                uPBOIndex = 0;
                bvalid = true;
                m_FPSCount = 0;
                m_PushFPSCount = 0;
                //pText = NULL;
                bDisplayText = false;
                r = 255.0f;
                g = 0.0f;
                b = 0.0f;
                xPos = 0.0f;
                yPos = 0.0f;
                ScaleSize = 1.0f;
                bPrintFps = false;
                bSelect = false;

                bTextMove = false;
                fRate = 0.0f;
                fXMove = 0.0f;
                fYMove = 0.0f;
                fTextSumWidth = 0.0f;
                bBGfalg = true;
                bAlreadyStart = false;
                //�ƶ��ķ�ʽ(��ֱ��ˮƽ�ƶ�)
                MovePolicy = enMovePolicy_NotMove;
                memset(pText,0,STR_LEN);
                bGetTextWidthflag = true;

                mtx_init(&m_mtxDeleteSubscreen,mtx_plain);
                cnd_init(&m_cndDeletSubscreen);
            }

            ~SSubSrceenData()
            {
                pDatas.Clear();

                //delete []pText;
                //pText = NULL;
                bDisplayText = false;
                bPrintFps = false;
                bvalid = false;
                bSelect = false;
                ID = -1;
                if (NULL != pPic)
                {
                    delete pPic;
                    pPic = NULL;
                }

                if (NULL != pSelectPic)
                {
                    delete pSelectPic;
                    pSelectPic = NULL;
                }
            }

            void GetFrameRate()
            {
                double elapsedTime = m_FPSTimer.getElapsedTime();
                if (elapsedTime < 1.0)
                {
                    ++m_FPSCount;
                }
                else
                {
                    LOG_IF(ERROR, m_FPSCount == 0)<<"subscreen id:"<<ID<<" no image data";
                    //LOG_DEBUG << "subscreen id :" << ID << ",fps:" << m_FPSCount;
                    m_FPSCount = 0;
                    m_FPSTimer.start();
                }
            }

            void Selected(int sharedID,GLfloat w,GLfloat h);
            void Selected(int sharedID);

            bool bvalid;
            unsigned int ID;
            SRect SubSrceenPos;
            //��������
            GLBatch* pPic;
			//ѡ������
            GLBatch* pSelectPic;
            bool bSelect;
			//ȫ��
			GLBatch* pFullPic;

            //��������Ļ��Ϣ
            WCHar pText[STR_LEN];
            GLfloat r;
            GLfloat g;
            GLfloat b;
            GLfloat ScaleSize;
            GLfloat xPos;
            GLfloat yPos;
            bool bPrintFps;
            bool bDisplayText;
            

            //����ID
            GLuint TextureID;
            GLuint uPBOs[2];
            GLuint uPBOIndex;

            //����ͼ����ID
            bool bBGfalg;
           
            //ÿ��������֡�����
            Timer m_FPSTimer;
            int m_FPSCount;

            Timer m_PushFPSTimer;
            int m_PushFPSCount;

            //��Ļ�ƶ����
            bool bTextMove;
            GLfloat fRate;
            GLfloat fXMove;
            GLfloat fYMove;
            //��Ļ���ܿ��
            GLfloat fTextSumWidth;
            //�ƶ��ķ�ʽ(��ֱ��ˮƽ�ƶ�)
            enMovePolicy MovePolicy;
            bool bGetTextWidthflag;

            //�ͷ����ݶ��е����������뻥����
            mtx_t m_mtxDeleteSubscreen;
            cnd_t m_cndDeletSubscreen;

            CPCDeque pDatas;
            bool bAlreadyStart;
        };

        typedef boost::shared_ptr<SSubSrceenData> SubScreenPtr;

        std::map<unsigned int,SubScreenPtr> m_mapWindowsDatas;
        int m_uSubSrceenID;

        /*GLuint m_uTexture[MAX_TEXTURE];*/
        GLFWwindow *m_pGlfWindow;
        GLFWwindow *m_pFullWindows;
        //Monitor��������
        GLFWmonitor *m_pMonitor;
        const GLFWvidmode* m_pMonitorMode;

        void *m_pParent;

        bool m_bStopDraw;

        //PBO����
        //GLuint m_uPBOIndex;
        GLuint m_uPBOs[32];
        GLuint m_uPBOIndex;

        mtx_t m_mutSubscreenList;

        //ȫ�����
        bool m_bEnableSelected;
        int m_iFullSubScreenID;
        bool m_bFullSubScreen;
        bool m_bIsCancelFullScreen;

        ENDrawMode m_enType;
        bool m_bAlreadySetSize;
        bool m_bAlreadyOldSize;

        //������Ļ�Ĵ�С
        unsigned int m_uScreenWidth;
        unsigned int m_uScreenHeight;

    private:
        int DeleteSubScreen(const SubScreenPtr &pData);

    private:
        CRenderTextManger m_RenderText;

    private:
        //�¼��ص����
        static CursorPosCallBack m_funCursorPos;
        static MouseButtonCallBack m_funMouseBtn;
};

#endif