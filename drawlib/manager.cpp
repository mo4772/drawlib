#include "manager.h"
#ifdef WIN32
#define LOGDIR ".\\log"
#endif
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
    LOG_DEBUG << "Enter the function CManager::Init()";
    if (!glfwInit())
    {
        LOG_ERROR <<"glfw init failed";
        return -1;
    }

    int iMonitorCnt = 0;
    GLFWmonitor **pMonitors = glfwGetMonitors(&iMonitorCnt);
    LOG_DEBUG << "get monitor cnt " << iMonitorCnt;
    for (int i = 0; i < iMonitorCnt; ++i)
    {
        SMonitorInfo monitorInfo;
        monitorInfo.iSeqNo = i;
        monitorInfo.pMonior = pMonitors[i];
        const GLFWvidmode *pVideoMode = glfwGetVideoMode(monitorInfo.pMonior);
        monitorInfo.iMaxWidth = pVideoMode->width;
        monitorInfo.iMaxHeight = pVideoMode->height;
        m_vecMonitorInfos.push_back(monitorInfo);

        LOG_DEBUG << "monitor num " << i << ",width " << pVideoMode->width << ",height " << pVideoMode->height;
    }

    mtx_init(&m_mutCreateWindow,mtx_plain);
    cnd_init(&m_cndCreateWindow);

    mtx_init(&m_mutCreateDraw,mtx_plain);
    cnd_init(&m_cndCreateDraw);

#ifdef WIN32
#ifndef USE_STATIC
    google::InitGoogleLogging("drawlib");
    google::SetLogDestination(google::GLOG_INFO, LOGDIR"/Drawlib_INFO_");
    google::SetLogDestination(google::GLOG_ERROR, LOGDIR"/Drawlib_ERROR_");
    google::SetStderrLogging(google::GLOG_INFO);
    FLAGS_logtostderr = false;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 100;
    FLAGS_stop_logging_if_full_disk = true;
#endif
#endif
    m_vecSubScreenID.reserve(MAX_SUBSRCEEN_CNT);
    m_vecSubScreenID.resize(MAX_SUBSRCEEN_CNT);

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

int CManager::GetMonitorCnt()
{
    return m_vecMonitorInfos.size();
}

int CManager::CreateVideoWindows(void *parent, SRect pos, const char* pBGFile, int iMonitorIndex, bool bFullScreen)
{
    LOG_DEBUG << "Enter the function CManager::CreateVideoWindows";
    LOG_DEBUG << "choose monitor index " << iMonitorIndex << " for display";
    LOG_DEBUG << "create window,x:" << pos.x << ",y:" << pos.y << ",width:" << pos.width << ",height:" << pos.height;
    if (m_mapVideoWindows.size() > 1)
    {
        LOG_ERROR<<"Not allow create more than one windows";
        return -1;
    }

    if (iMonitorIndex <0 || iMonitorIndex >= m_vecMonitorInfos.size())
    {
        LOG_ERROR << "invalid monitor index " << iMonitorIndex;
        return -1;
    }

    SWindowData *pData = new SWindowData;
    pData->pos = pos;
    pData->bFullScreen = bFullScreen;
    pData->pDraw = new CDraw(m_vecMonitorInfos[iMonitorIndex].pMonior);
    m_iCurrUseMonitorSeq = iMonitorIndex;
#ifdef WIN32
    pData->pParent = (HWND*)parent;
#endif
    pData->pDraw->SetScreenWH(pos.width, pos.height);
    if (NULL != pBGFile)
    {
        pData->pDraw->m_strBG = pBGFile;
    }

    m_mapVideoWindows.insert(std::make_pair(m_iWndID, pData));

    Start();
    LOG_DEBUG << "Exit the function CManager::CreateVideoWindows";
    return m_iWndID++;
}
    
int CManager::CreateVideoWindows(SRect pos,const char* pBGFile,int iMonitorIndex)
{
    LOG_DEBUG << "Enter the function CManager::CreateVideoWindows";
    LOG_DEBUG << "choose monitor num " << iMonitorIndex << " for display";
    if (m_mapVideoWindows.size() > 1)
    {
        return -1;
    }

    if (iMonitorIndex <0 || iMonitorIndex >= m_vecMonitorInfos.size())
    {
        LOG_ERROR << "invalid monitor index " << iMonitorIndex;
        return -1;
    }

    SWindowData *pData = new SWindowData;
    pData->pos = pos;
    pData->bFullScreen = true;
    pData->pDraw = new CDraw(m_vecMonitorInfos[iMonitorIndex].pMonior);
    m_iCurrUseMonitorSeq = iMonitorIndex;

#ifdef WIN32
    pData->pParent = NULL;
#endif
    pData->pDraw->SetScreenWH(pos.width,pos.height);
    if (NULL != pBGFile)
    {
        pData->pDraw->m_strBG = pBGFile;
    }
    
    m_mapVideoWindows.insert(std::make_pair(m_iWndID,pData));

    StartWithFull();
    LOG_DEBUG << "Exit the function CManager::CreateVideoWindows";
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

int CManager::ChangeSubscreenPos(int iWindID,int iSubScreenID, SRect Pos)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWindID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    SWindowData::SSubScreenPosPtr pSubData(new SWindowData::SSubScreenData);
    pSubData->iSubSrceenID = iSubScreenID;
    pSubData->rect = Pos;

    mtx_lock(&FindIt->second->m_mutCreateScreen);
    FindIt->second->deqSubSrceenPos.push_back(pSubData);
    mtx_unlock(&FindIt->second->m_mutCreateScreen);

    cnd_wait(&pSubData->cndCreateScreen, &pSubData->mtxCreateScreen);
    
    return 0;
}

int CManager::CreateFullScreen(int iWindID)
{
    LOG_DEBUG << "Enter the function CManager::CreateFullScreen";
    std::map<int, SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWindID);
    if (FindIt == m_mapVideoWindows.end())
    {
        LOG_ERROR << "can't find window with windid " << iWindID;
        return -1;
    }

    SWindowData::SSubScreenPosPtr pSubData(new SWindowData::SSubScreenData);

    bool bHaveID = false;
    //分配视口ID
    for (int i = 0; i < MAX_SUBSRCEEN_CNT; ++i)
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

    SRect pos;
    pos.x = 0;
    pos.y = 0;
    pos.width = m_vecMonitorInfos[m_iCurrUseMonitorSeq].iMaxWidth;
    pos.height = m_vecMonitorInfos[m_iCurrUseMonitorSeq].iMaxHeight;

    pSubData->rect = pos;

    mtx_lock(&FindIt->second->m_mutCreateScreen);
    FindIt->second->deqSubSrceenPos.push_back(pSubData);
    mtx_unlock(&FindIt->second->m_mutCreateScreen);

    cnd_wait(&pSubData->cndCreateScreen, &pSubData->mtxCreateScreen);

    LOG_DEBUG << "Exit the function CManager::CreateFullScreen";
    return pSubData->iSubSrceenID;
}

int CManager::CreateSubScreen(int winID,SRect pos)
{
    LOG_DEBUG << "Enter the function CManager::CreateSubScreen";
    LOG_DEBUG << "create subscreen,x:" << pos.x << ",y:" << pos.y << ",width:" << pos.width << ",height:" << pos.height;
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        LOG_ERROR << "can't find window with winid " << winID;
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
        LOG_ERROR << "Already create max cnt subscreen";
        return -1;
    }

    pSubData->rect = pos;

    mtx_lock(&FindIt->second->m_mutCreateScreen);
    FindIt->second->deqSubSrceenPos.push_back(pSubData);
    mtx_unlock(&FindIt->second->m_mutCreateScreen);

    cnd_wait(&pSubData->cndCreateScreen,&pSubData->mtxCreateScreen);

    LOG_DEBUG << "Exit the function CManager::CreateSubScreen";
    return pSubData->iSubSrceenID;
}

int CManager::DeleteSubScreen(int iWinID,int iSubScreenID)
{
    LOG_DEBUG << "Enter the function CManager::DeleteSubScreen";
    LOG_DEBUG << "delete subscreen,id " << iSubScreenID;

    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        LOG_ERROR << "can't find window with winid " << iWinID;
        return -1;
    }

    //回收视口ID
    if (iSubScreenID <0 || iSubScreenID > MAX_SUBSRCEEN_CNT)
    {
        LOG_ERROR << "invalid subcreen id "<< iSubScreenID;
        return -1;
    }

    m_vecSubScreenID[iSubScreenID] = 0;
    LOG_DEBUG << "Enter the function CManager::DeleteSubScreen";
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
        return -1;
    }

    return FindIt->second->pDraw->UpdateSubScreenImage(iScreenID,pData,iWidth,iHeight,iSize);
}

int CManager::SetSubScreenVideoText(int winID,int iScreenID,SVideoTextInfo *pTextInfo)
{
    LOG_DEBUG << "Enter the function CManager::SetSubScreenVideoText";
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return -1;
    }

    LOG_DEBUG << "Exit the function CManager::SetSubScreenVideoText";
    return FindIt->second->pDraw->SetSubScreenVideoText(iScreenID,pTextInfo);
}

int CManager::DisableSubScreenVideoText(int winID,int iScreenID)
{
    LOG_DEBUG << "Enter the function CManager::DisableSubScreenVideoText";
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(winID);
    if (FindIt == m_mapVideoWindows.end())
    {
        //ERROR("UpdateImageData,can't find winid %d",winID);
        return -1;
    }

    LOG_DEBUG << "Exit the function CManager::DisableSubScreenVideoText";
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

int CManager::WindThreadForFull(void *arg)
{
    LOG_DEBUG << "=======>Start WindThreadForFull thread";
    CManager *pData = (CManager*)arg;

    //创建主窗口
    std::map<int, SWindowData*>::iterator SIt = pData->m_mapVideoWindows.begin();
    for (; SIt != pData->m_mapVideoWindows.end(); ++SIt)
    {
#ifdef WIN32
       
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pos))
#else
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pos))
#endif
        {
            //ERROR("Create video srceen failed");
            return -1;
        }
    }

    cnd_signal(&(pData->m_cndCreateDraw));
    while (pData->m_bRunning)
    {
        std::map<int, SWindowData*>::iterator It = pData->m_mapVideoWindows.begin();
        for (; It != pData->m_mapVideoWindows.end(); ++It)
        {
            if (NULL == It->second->pDraw)
            {
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

    LOG_DEBUG << "=======>End WindThreadForFull thread";
    return 0;
}

int CManager::WindThread(void *arg)
{
    LOG_DEBUG << "======>Start WindThread";
    CManager *pData = (CManager*)arg;

    //创建主窗口
    std::map<int,SWindowData*>::iterator SIt = pData->m_mapVideoWindows.begin();
    for (; SIt != pData->m_mapVideoWindows.end(); ++SIt)
    {
#ifdef WIN32
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pParent,SIt->second->pos,SIt->second->bFullScreen))
        //if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pos))
#else
        if (-1 == SIt->second->pDraw->CreateVideoSrceen(NULL,SIt->second->pos,SIt->second->bFullScreen))
        //if (-1 == SIt->second->pDraw->CreateVideoSrceen(SIt->second->pos))
#endif
        {
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
    
    LOG_DEBUG << "======>End CManager::WindThread";
    return 0;
}

int CManager::DrawThread(void *arg)
{
    LOG_DEBUG << "======>Start Draw thread";
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

    LOG_DEBUG << "======>End Draw thread";
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

int CManager::StartWithFull()
{
    thrd_create(&m_ManagerThrdID, WindThreadForFull, this);
    //等待窗口创建完毕
    cnd_wait(&m_cndCreateDraw, &m_mutCreateDraw);
    std::map<int, SWindowData*>::iterator SIt = m_mapVideoWindows.begin();
    for (; SIt != m_mapVideoWindows.end(); ++SIt)
    {
        thrd_create(&SIt->second->threadID, DrawThread, SIt->second);
        cnd_wait(&SIt->second->m_cndSubScreen, &SIt->second->m_mutSubScreen);
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
        return ;
    }

    FindIt->second->pDraw->SelectSubScreen(iSubScreen);
}

void CManager::SetFullScreen(int iWinID,int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return ;
    }

    FindIt->second->pDraw->SetFullScreen(iSubScreenID);
}

void CManager::CancelFullScreen(int iWinID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return ;
    }

    FindIt->second->pDraw->CancelFullScreen();
}

void CManager::SetTextMoveInfo(int iWinID,int SubScreenID,enMovePolicy enPolicy,float fRate)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return ;
    }

    FindIt->second->pDraw->SetTextMoveInfo(SubScreenID,enPolicy,fRate);
}

void CManager::StartPlay(int iWinID,int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return ;
    }

    FindIt->second->pDraw->StartDrawVideo(iSubScreenID);
}

void CManager::StopPlay(int iWinID, int iSubScreenID)
{
    std::map<int,SWindowData*>::iterator FindIt = m_mapVideoWindows.find(iWinID);
    if (FindIt == m_mapVideoWindows.end())
    {
        return ;
    }

    FindIt->second->pDraw->StopDrawVideo(iSubScreenID);
}

