#include "manager.h"
#include "log.h"

CManager* CManager::m_pInstance = NULL;

CManager* CManager::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CManager;
    }

    return m_pInstance;
}

void CManager::Release()
{
    if (NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

int CManager::Init()
{
//     google::InitGoogleLogging("drawlib");
//     //FLAGS_stderrthreshold = google::GLOG_INFO;
//     //FLAGS_colorlogtostderr = true;
//     //FLAGS_log_dir = ".";
//     google::SetLogDestination(google::GLOG_INFO, "./log/drawlib.log");
//     FLAGS_logbuflevel = -1;

    if (!glfwInit())
    {
        return -1;
    }

    mtx_init(&m_mutCreateWindow,mtx_plain);
    cnd_init(&m_cndCreateWindow);

    mtx_init(&m_mutCreateDraw,mtx_plain);
    cnd_init(&m_cndCreateDraw);

    m_vecSubScreenID.reserve(MAX_SUBSRCEEN_CNT);
    m_vecSubScreenID.resize(MAX_SUBSRCEEN_CNT);

    //初始化日志
//     std::string strLogPath = ".\\";
//     std::string strMoudleName = "drawlib";
//     INIT_LOG(strLogPath,strMoudleName);
//     CLog::GetInstance()->SetLogLevel(LogLevelDebug);

    //DEBUG("======>Init success");

    return 0;
}

CManager::CManager()
{
    m_bRunning = true;
    m_iWndID = 0;
    m_iGenerSubScreenID = 0;
}

CManager::~CManager()
{
    m_bRunning = false;
    thrd_join(m_ManagerThrdID,NULL);

    std::map<int,SWindowData*>::iterator it = m_mapVideoWindows.begin();
    for (; it != m_mapVideoWindows.end(); ++it)
    {
        it->second->bNeedToClose = true;
        thrd_join(it->second->threadID,NULL);

        delete it->second->pDraw;
        it->second->pDraw = NULL;

        it->second->deqSubSrceenPos.clear();
    }
}

int CManager::CreateVideoWindows(void *parent,SRect pos,const char* pBGFile,bool bFullScreen)
{
    //DEBUG("Create window pos(x:%d,y:%d,w:%d,h:%d)",pos.x,pos.y,pos.width,pos.height);
    if (m_mapVideoWindows.size() > 1)
    {
        //ERROR("Not allow create more than one windows");
        return -1;
    }

    SWindowData *pData = new SWindowData;
    pData->pos = pos;
    pData->bFullScreen = bFullScreen;
    pData->pDraw = new CDraw;
#ifdef WIN32
    pData->pParent = (HWND*)parent;
#endif
    pData->pDraw->SetScreenWH(pos.width,pos.height);
    if (NULL != pBGFile)
    {
        pData->pDraw->m_strBG = pBGFile;
    }
    
    m_mapVideoWindows.insert(std::make_pair(m_iWndID,pData));

    Start();
    //DEBUG("Return window with id %d",m_iWndID);
    return m_iWndID++;
}

int CManager::SetVideoWindowSize(int iWinID,int iWidth,int iHeight)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    return FindIt->second->pDraw->SetWindowSize(iWidth,iHeight);
}

int CManager::SetVideoWindowPos(int iWindID,int x,int y)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWindID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    FindIt->second->pDraw->SetWindowPos(x,y);
    return 0;
}

int CManager::ChangeSubscreenSize(int iWindID,int iSubScreenID,int iWidth,int iHeight)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWindID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    FindIt->second->pDraw->ChangeSubscreenSize(iSubScreenID,iWidth,iHeight);
    return 0;
}

int CManager::CreateSubScreen(int winID,SRect pos)
{
    //DEBUG("CreateSubScreen winid is %d,pos(x:%d,y:%d,w:%d,h:%d)",winID,pos.x,pos.y,pos.width,pos.height);

    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("Can;t find winid %d",winID);
        return -1;
    }

    SWindowData::SSubScreenPosPtr pSubData(new SWindowData::SSubScreenData);

    bool bHaveID = false;
    //分配视口ID
    for (int i=0; i<MAX_SUBSRCEEN_CNT; ++i)
    {
        if (0 == m_vecSubScreenID[i])
        {
            pSubData->iSubSrceenID = i;
            m_vecSubScreenID[i] = 1;
            bHaveID = true;
            break;
        }
    }
    
    if (!bHaveID)
    {
        //ERROR("Already create max cnt subscreen");
        return -1;
    }

    pSubData->rect = pos;

    mtx_lock(&FindIt->second->m_mutCreateScreen);
    FindIt->second->deqSubSrceenPos.push_back(pSubData);
    mtx_unlock(&FindIt->second->m_mutCreateScreen);

    cnd_wait(&pSubData->cndCreateScreen,&pSubData->mtxCreateScreen);

    /*++m_iGenerSubScreenID;*/
    //DEBUG("The subsrceen id is %d",pSubData->iSubSrceenID);
    return pSubData->iSubSrceenID;
}

int CManager::DeleteSubScreen(int iWinID,int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    //回收视口ID
    if (iSubScreenID <0 || iSubScreenID > MAX_SUBSRCEEN_CNT)
    {
        return -1;
    }

    m_vecSubScreenID[iSubScreenID] = 0;
    return FindIt->second->pDraw->DeleteSubScreen(iSubScreenID);
}

int CManager::UpdateImageData(int iWinID,
                              int iScreenID,
                              unsigned char* pData,
                              unsigned int iWidth,
                              unsigned int iHeight,
                              unsigned int iSize)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return -1;
    }

    return FindIt->second->pDraw->UpdateSubScreenImage(iScreenID,pData,iWidth,iHeight,iSize);

}

int CManager::SetSubScreenVideoText(int winID,int iScreenID,SVideoTextInfo *pTextInfo)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",winID);
        return -1;
    }

    return FindIt->second->pDraw->SetSubScreenVideoText(iScreenID,pTextInfo);
}

int CManager::DisableSubScreenVideoText(int winID,int iScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",winID);
        return -1;
    }

    return FindIt->second->pDraw->DisableSubScreenVideoText(iScreenID);
}

void CManager::PrintFrameRate(int winID,int iScreenID,bool bEnable)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",winID);
        return;
    }

    return FindIt->second->pDraw->PrintFrameRate(iScreenID,bEnable);
}

int CManager::WindThread(void *arg)
{
    //DEBUG("=====>Start windThread");
    CManager *pData = (CManager*)arg;

    //创建主窗口
    std::map<int,SWindowData*>::iterator SIt = pData->m_mapVideoWindows.begin();
    for (; SIt != pData->m_mapVideoWindows.end(); ++SIt)
    {
#ifdef WIN32
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pParent,SIt->second->pos,SIt->second->bFullScreen))
#else
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(NULL,SIt->second->pos,SIt->second->bFullScreen))
#endif
        {
            //ERROR("Create video srceen failed");
            return -1;
        }
    }

    cnd_signal(&(pData->m_cndCreateDraw));
    while (pData->m_bRunning)
    {
        std::map<int,SWindowData*>::iterator It = pData->m_mapVideoWindows.begin();
        for (; It != pData->m_mapVideoWindows.end();++It)
        {
            if (NULL == It->second->pDraw)
            {
                //ERROR("win id %d draw pointer is NULL",It->first);
                continue;
            }
            
            if (!It->second->bNeedToClose && glfwWindowShouldClose(It->second->pDraw->GetVideoSrceenHanlde()))
            {//释放该窗口的资源
                It->second->bNeedToClose = true;
                glfwDestroyWindow(It->second->pDraw->GetVideoSrceenHanlde());
            }
        }

        glfwWaitEvents();
    }
    
    //DEBUG("=====>End windThread");
    return 0;
}

int CManager::DrawThread(void *arg)
{
    //DEBUG("======>Start DrawThread");
    SWindowData *pData = (SWindowData*)arg;
    if (-1 == pData->pDraw->PrepareDrawPic())
    {
        return -1;
    }

    mtx_lock(&pData->m_mutCreateScreen);
    std::deque<SWindowData::SSubScreenPosPtr> &dequePos = pData->deqSubSrceenPos;
    for (int i=0; i<dequePos.size(); ++i)
    {
        if (-1 == pData->pDraw->CreateSubScreen(dequePos[i]->rect,dequePos[i]->iSubSrceenID))
        {
            cnd_signal(&dequePos[i]->cndCreateScreen);
            continue;
        }

        cnd_signal(&dequePos[i]->cndCreateScreen);
    }

    mtx_unlock(&pData->m_mutCreateScreen);
    
    dequePos.clear();
    cnd_signal(&pData->m_cndSubScreen);

    while (!pData->bNeedToClose)
    {
        mtx_lock(&pData->m_mutCreateScreen);
        if (!pData->deqSubSrceenPos.empty())
        {
            SWindowData::SSubScreenPosPtr subData = dequePos.front();
            pData->deqSubSrceenPos.pop_front();
            if (-1 == pData->pDraw->CreateSubScreen(subData->rect,subData->iSubSrceenID))
            {
                cnd_signal(&subData->cndCreateScreen);
                continue;
            }

            cnd_signal(&subData->cndCreateScreen);
        }
        mtx_unlock(&pData->m_mutCreateScreen);

        //glfwSwapInterval(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //bool flag = pData->pDraw->ChangeToFullDraw();
        //if (!flag)
        {
            pData->pDraw->DrawPic();
        }

        glfwSwapBuffers(pData->pDraw->GetVideoSrceenHanlde());
    }

    //DEBUG("======>End DrawThread");
    return 0;
}

int CManager::Start()
{
    thrd_create(&m_ManagerThrdID,WindThread,this);
    //等待窗口创建完毕
    cnd_wait(&m_cndCreateDraw,&m_mutCreateDraw);

    std::map<int,SWindowData*>::iterator SIt = m_mapVideoWindows.begin();
    for (; SIt != m_mapVideoWindows.end(); ++SIt)
    {
        thrd_create(&SIt->second->threadID,DrawThread,SIt->second);
        cnd_wait(&SIt->second->m_cndSubScreen,&SIt->second->m_mutSubScreen);
    }

    return 0;
}

void CManager::SetMousBtnActionCallback(int iWinID,MouseButtonCallBack f)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return;
    }

    FindIt->second->pDraw->SetMouseBtnCallback(f);
}

void CManager::SetCursorCallback(int iWinID,CursorPosCallBack f)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return;
    }

    FindIt->second->pDraw->SetCursorCallback(f);
}

int CManager::GetSelectSubScreen(int iWinID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return -1;
    }

    return FindIt->second->pDraw->GetSelectSubScreen();
}

void CManager::SelectSubScreen(int iWinID,int iSubScreen)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->SelectSubScreen(iSubScreen);
}

void CManager::SetFullScreen(int iWinID,int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->SetFullScreen(iSubScreenID);
}

void CManager::CancelFullScreen(int iWinID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->CancelFullScreen();
}

void CManager::SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->SetTextMoveInfo(SubScreenID,enPolicy,fRate);
}

void CManager::StartPlay(int iWinID,int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->StartDrawVideo(iSubScreenID);
}

void CManager::StopPlay(int iWinID, int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",iWinID);
        return ;
    }

    FindIt->second->pDraw->StopDrawVideo(iSubScreenID);
}

