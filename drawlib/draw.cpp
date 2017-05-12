#include <ctime>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "draw.h"
#include "log.h"
#include "SOIL.h"

//定点坐标
static GLfloat sVerts[] = {-1.0f,1.0f,0.0f, 
                           1.0f,1.0f,0.0f,
                           -1.0f,-1.0f,0.0f,
                           1.0f,-1.0f,0.0f};


static GLfloat SelectVerts[] = {-1.0f,1.0f,0.0f,
                                1.0f,1.0f,0.0f,
                                1.0f,-1.0f,0.0f,
                                -1.0f,-1.0f,0.f};

//纹理坐标
static GLfloat sTexture[] = {0.0f,0.0f,
							1.0f,0.0f,
							0.0f,1.0f,
							1.0f,1.0f};

static GLfloat fsTexture[] = {0.0f,1.0f,
							 1.0f,1.0f,
							 0.0f,0.0f,
							 1.0f,0.0f};

static GLuint Textures[16] = {0};



void CDraw::SSubSrceenData::Selected(int sharedID,GLfloat w,GLfloat h)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform3f(glGetUniformLocation(sharedID, "textColor"), 255.0f, 0.0f, 0.0f);
	glUniform1i(glGetUniformLocation(sharedID, "vselectflag"),GL_TRUE);
	glUniform1i(glGetUniformLocation(sharedID, "fselectflag"),GL_TRUE);

	M3DMatrix44f mScreenSpace;
	m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, w, -1.0f*h, 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(ID, "mvpMatrix"), 1, GL_FALSE, mScreenSpace);

	pSelectPic->Draw(sharedID);
	glUniform1i(glGetUniformLocation(sharedID, "vselectflag"),GL_FALSE);
	glUniform1i(glGetUniformLocation(sharedID, "fselectflag"),GL_FALSE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void CDraw::SSubSrceenData::Selected(int sharedID)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform3f(glGetUniformLocation(sharedID, "textColor"), 255.0f, 0.0f, 0.0f);
    glUniform1i(glGetUniformLocation(sharedID, "vselectflag"),GL_TRUE);
    glUniform1i(glGetUniformLocation(sharedID, "fselectflag"),GL_TRUE);

    pSelectPic->Draw(sharedID);
    glUniform1i(glGetUniformLocation(sharedID, "vselectflag"),GL_FALSE);
    glUniform1i(glGetUniformLocation(sharedID, "fselectflag"),GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

CursorPosCallBack CDraw::m_funCursorPos = NULL;
MouseButtonCallBack CDraw::m_funMouseBtn = NULL;

CDraw::CDraw() :m_uSubSrceenID(0),
                m_bStopDraw(false),
                m_pGlfWindow(NULL),
                m_iFullSubScreenID(-1),
                m_bFullSubScreen(false),
                m_bEnableSelected(false),
                m_pFullWindows(NULL),
                m_enType(ENDrawMode_WINDOWS),
                m_uPBOIndex(0),
                m_bAlreadySetSize(false),
                m_bAlreadyOldSize(false),
                m_bIsCancelFullScreen(false),
                m_uScreenWidth(0),
                m_uScreenHeight(0),
                m_uBGTextureID(0),
                m_pMonitor(NULL)
{
    memset(m_uPBOs, 0, 2 * sizeof(GLuint));

    mtx_init(&m_mutSubscreenList, mtx_plain);
}

CDraw::CDraw(GLFWmonitor * pMonitor):
               m_uSubSrceenID(0),
               m_bStopDraw(false),
               m_pGlfWindow(NULL),
               m_iFullSubScreenID(-1),
               m_bFullSubScreen(false),
               m_bEnableSelected(false),
               m_pFullWindows(NULL),
               m_enType(ENDrawMode_WINDOWS),
               m_uPBOIndex(0),
               m_bAlreadySetSize(false),
               m_bAlreadyOldSize(false),
               m_bIsCancelFullScreen(false),
               m_uScreenWidth(0),
               m_uScreenHeight(0),
               m_uBGTextureID(0),
               m_pMonitor(pMonitor)
{
    memset(m_uPBOs,0,2*sizeof(GLuint));
    
    mtx_init(&m_mutSubscreenList,mtx_plain);
}

CDraw::~CDraw()
{
    mtx_destroy(&m_mutSubscreenList);
}

int CDraw::Init()
{
    return 0;
}

int CDraw::UpdateSubScreenImage(unsigned int ID,
                                unsigned char* pData,
                                unsigned int iWidth,
                                unsigned int iHeigth,
                                unsigned int iSize)
{
    std::clock_t start = clock();

    std::map<unsigned int,SubScreenPtr>::iterator It = m_mapWindowsDatas.find(ID);
    if (It == m_mapWindowsDatas.end())
    {
        return -1;
    }
    
    //LOG_DEBUG << "+++ UpdateSubScreenImage " << ID;
    SubScreenPtr &pSubScreen = It->second;
    double elapsedTime = pSubScreen->m_PushFPSTimer.getElapsedTime();
    if (elapsedTime < 1.0)
    {
        ++pSubScreen->m_PushFPSCount;
    }
    else
    {
        //LOG_IF(ERROR, m_FPSCount == 0)<<"subscreen id:"<<ID<<" no image data";
        //LOG_DEBUG << "++++++++++++subscreen id :" << ID << ",fps:" << pSubScreen->m_PushFPSCount;
        pSubScreen->m_PushFPSCount = 0;
        pSubScreen->m_PushFPSTimer.start();
    }

    if (!pSubScreen->bBGfalg)
    {//如果是画背景图，则不会往队列中放数据
        SVideoData *pVideoData = new SVideoData;
        pVideoData->iSize = iSize;

        pVideoData->pData.reset(new unsigned char[iSize]);
        memcpy(pVideoData->pData.get(),pData,iSize);

        pVideoData->iWidth = iWidth;
        pVideoData->iHeight = iHeigth;

        DataPtr Data;
        Data.reset(pVideoData);
        pSubScreen->pDatas.PutData(Data);
    }

    std::clock_t end = clock();
    //LOG_DEBUG << "cost " << (double)(end - start) / CLOCKS_PER_SEC << "s";
    return 0;
}

GLFWwindow* CDraw::GetVideoSrceenHanlde()
{
    return m_pGlfWindow;
}

bool CDraw::isStartVideo(unsigned int iSubSrceenID)
{
    std::map<unsigned int, SubScreenPtr>::iterator It = m_mapWindowsDatas.find(iSubSrceenID);
    if (It == m_mapWindowsDatas.end())
    {
        return false;
    }

    return It->second->bAlreadyStart;
}

void CDraw::StartDrawVideo(unsigned int iSubscrrenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator It = m_mapWindowsDatas.find(iSubscrrenID);
    if (It == m_mapWindowsDatas.end())
    {
        LOG_ERROR << "can't find subscreen id " << iSubscrrenID;
        return ;
    }
    
    It->second->bBGfalg = false;
    It->second->bAlreadyStart = true;
}

void CDraw::StopDrawVideo(unsigned int iSubscrrenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator It = m_mapWindowsDatas.find(iSubscrrenID);
    if (It == m_mapWindowsDatas.end())
    {
        LOG_ERROR << "can't find subscreen id " << iSubscrrenID;
        return ;
    }

    It->second->bBGfalg = true;
    It->second->bAlreadyStart = false;
}

int CDraw::CreateVideoSrceen(SRect rect)
{
    LOG_DEBUG << "Enter the function CDraw::CreateVideoSrceen";
    if (NULL == m_pMonitor)
    {
        LOG_ERROR << "can't get monitor for display";
        return -1;
    }

    m_pMonitorMode = glfwGetVideoMode(m_pMonitor);
    m_VideoSrceenRect.x = 0;
    m_VideoSrceenRect.y = 0;
    m_VideoSrceenRect.width = m_pMonitorMode->width;
    m_VideoSrceenRect.height = m_pMonitorMode->height;

    unsigned int uWidth = m_VideoSrceenRect.width;
    unsigned int uHeight = m_VideoSrceenRect.height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
    glfwSetErrorCallback(ErrorInfo);

    m_pGlfWindow = m_pFullWindows = glfwCreateWindow(m_pMonitorMode->width, m_pMonitorMode->height, "", m_pMonitor, NULL);
    glfwSetWindowSizeCallback(m_pGlfWindow, ChangeSizeFun);
    glfwSetWindowUserPointer(m_pGlfWindow, this);
    LOG_DEBUG << "Exit the function CDraw::CreateVideoSrceen";

    return 0;
}

//获取该对象对应的窗口句柄
int CDraw::CreateVideoSrceen(void *parent,SRect rect,bool bFullScreen)
{
    LOG_DEBUG<<"Enter the function CDraw::CreateVideoSrceen1";
    LOG_DEBUG << "is full screen " << bFullScreen;

    if (NULL == m_pMonitor)
    {
        LOG_ERROR << "can't get monitor for display";
        return -1;
    }

    m_pMonitorMode = glfwGetVideoMode(m_pMonitor);
    if (bFullScreen)
    {
        m_VideoSrceenRect.x = 0;
        m_VideoSrceenRect.y = 0;
        m_VideoSrceenRect.width = m_pMonitorMode->width;
        m_VideoSrceenRect.height = m_pMonitorMode->height;
    }
    else
    {
        m_VideoSrceenRect = rect;
    }
    
    m_pParent = parent;

    unsigned int uWidth = m_VideoSrceenRect.width;
    unsigned int uHeight = m_VideoSrceenRect.height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER,GL_TRUE);
    
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
    glfwSetErrorCallback(ErrorInfo);

    if (bFullScreen)
    {
        LOG_DEBUG << "create full windows," << "width " << uWidth << ",height " << uHeight;
        if (!m_pMonitor)
        {
            LOG_DEBUG << "monitor point is NULL";
        }

        LOG_DEBUG << "monitor mode width " << m_pMonitorMode->width << ",height " << m_pMonitorMode->height;
        m_pGlfWindow = m_pFullWindows = glfwCreateWindow(m_pMonitorMode->width,m_pMonitorMode->height,"",m_pMonitor,NULL);
    }
    else
    {
        m_pGlfWindow = glfwCreateWindow(uWidth,uHeight,"", NULL, NULL);
        if (!m_pGlfWindow)
        {
            LOG_ERROR<<"glfwCreateWindow return failed";
            return -1;
        }

        if (NULL != m_pParent)
        {
#ifdef WIN32
            HWND pWnd = (HWND)(m_pParent);
            HWND cWnd = glfwGetWin32Window(m_pGlfWindow);
            ::SetParent(cWnd,pWnd);
#endif
        }
    }

    glfwSetWindowSizeCallback(m_pGlfWindow,ChangeSizeFun);
    glfwSetWindowUserPointer(m_pGlfWindow,this);

    LOG_DEBUG<<"Exit the function CDraw::CreateVideoSrceen1";
    return 0;
}

int CDraw::CreateSubScreen(SRect rect,int SubSrceenID)
{
	LOG_DEBUG<<"Enter the function CDraw::CreateSubScreen";
    SubScreenPtr pData(new SSubSrceenData);

	std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubSrceenID);
    if (FindIt != m_mapWindowsDatas.end())
    {//如果已经有了则删掉之
        pData->bBGfalg = FindIt->second->bBGfalg;
        LOG_DEBUG << "Already exsit subsrceen id" << FindIt->first << ",delete it";
        DeleteSubScreen(FindIt->second);
        m_mapWindowsDatas.erase(FindIt);
	}

	LOG_DEBUG<<"CreateSubScreen with id "<<SubSrceenID;
	
	if (m_uBGTextureID > 0)
	{
		pData->bBGfalg = true;
	}

	pData->ID = SubSrceenID;
	pData->TextureID = Textures[pData->ID];
	glBindTexture(GL_TEXTURE_2D,Textures[pData->ID]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D,0);

	int iMaxSize = 1920*1080*3;

	glGenBuffers(2,pData->uPBOs);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pData->uPBOs[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, iMaxSize, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pData->uPBOs[1]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER,iMaxSize,0,GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

	rect.y = -1 * rect.y;
	rect.height = -1 * rect.height;
	pData->SubSrceenPos = rect;

	pData->fXMove = rect.width;


	//选中框
	GLfloat SelectRect[] = {
		rect.x,rect.y + rect.height,0.0f,
		rect.x + rect.width,rect.y + rect.height,0.0f,
		rect.x + rect.width,rect.y,0.0f,
		rect.x,rect.y,0.0f,
	};

	pData->pSelectPic = new GLBatch;
	pData->pSelectPic->Begin(GL_LINE_LOOP,4);
	pData->pSelectPic->CopyVertexData3f(SelectRect);
	pData->pSelectPic->End();

	//贴图框
	GLfloat vertices[] = 
	{
		rect.x,rect.y + rect.height,0.0f,         
		rect.x + rect.width, rect.y + rect.height,0.0f,
		rect.x, rect.y,0.0f,
		rect.x + rect.width,rect.y,0.0f,        
	};


	pData->pPic = new GLBatch;

	pData->pPic->Begin(GL_TRIANGLE_STRIP,4,1);
	pData->pPic->CopyVertexData3f(vertices);
	pData->pPic->CopyTexCoordData2f(fsTexture,0);
	pData->pPic->End();

	//全屏
	GLfloat FullVertices[] = 
	{
		0.0f,m_pMonitorMode->height,0.0f,         
		m_pMonitorMode->width, m_pMonitorMode->height,0.0f,
		0.0f,0.0f,0.0f,
		m_pMonitorMode->width,0.0f,0.0f,
	};

	pData->pFullPic = new GLBatch;
	pData->pFullPic->Begin(GL_TRIANGLE_STRIP,4,1);
	pData->pFullPic->CopyVertexData3f(FullVertices);
	pData->pFullPic->CopyTexCoordData2f(sTexture,0);
	pData->pFullPic->End();


	m_mapWindowsDatas.insert(std::make_pair(pData->ID,pData));

	LOG_DEBUG<<"Exit the function CDraw::CreateSubScreen";
	return 0;
}

int CDraw::DeleteSubScreen(int SubScreenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubScreenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return -1;
    }
    
    FindIt->second->bvalid = false;
    return 0;
}

int CDraw::SetSubScreenVideoText(int SubsrceenID,SVideoTextInfo *pInfo)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubsrceenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return -1;
    }

//     if (NULL != FindIt->second->pText)
//     {
//         delete[] FindIt->second->pText;
//         FindIt->second->pText = NULL;
//     }

    memset(FindIt->second->pText,0,STR_LEN);

#ifdef WIN32
    size_t iLen = wcslen(pInfo->szText);
    //FindIt->second->pText = new WCHar[iLen + 1];
    //memset(FindIt->second->pText,0,iLen + 1);
    wcsncpy(FindIt->second->pText,pInfo->szText,iLen);
#endif

    FindIt->second->r = (GLfloat)pInfo->color.R;
    FindIt->second->g = (GLfloat)pInfo->color.G;
    FindIt->second->b = (GLfloat)pInfo->color.B;
    if (ENTextSize_Big == pInfo->textSize)
    {
        FindIt->second->ScaleSize = 1.0f;
    }
    else if (ENTextSize_Normal == pInfo->textSize)
    {
        FindIt->second->ScaleSize = 0.7f;
    }
    else if (ENTextSize_Small == pInfo->textSize)
    {
        FindIt->second->ScaleSize = 0.5f;
    }

    FindIt->second->xPos = (GLfloat)pInfo->x;
    FindIt->second->yPos = (GLfloat)pInfo->y;

    /*m_RenderText.GetTextSumWidth(FindIt->second->pText,FindIt->second->ScaleSize,FindIt->second->fTextSumWidth);*/
    FindIt->second->bDisplayText = true;
    FindIt->second->bGetTextWidthflag = true;
    return 0;
}

int CDraw::GetFrameRate(int SubsrceenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubsrceenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return -1;
    }

    return FindIt->second->m_FPSCount;
}

void CDraw::PrintFrameRate(int SubsrceenID,bool bEnable)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubsrceenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return;
    }

    FindIt->second->bPrintFps = bEnable;
    
//     if (bEnable)
//     {
//         int FPS = FindIt->second->m_FPSCount;
//         char szBuf[128] = {0};
//         sprintf(szBuf,"FPS:%d/s",FPS);
// 
//         SVideoTextInfo info;
//         info.color.R = 255;
//         info.color.G = 0;
//         info.color.B = 0;
//         info.x = 0;
//         info.y = 0;
// #ifdef WIN32
//         wcsncpy(info.szText,(WCHar*)szBuf,strlen(szBuf));
// #endif
//         info.textSize = ENTextSize_Small;
//     }
}

int CDraw::DisableSubScreenVideoText(int SubsrceenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubsrceenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return -1;
    }

    FindIt->second->bDisplayText = false;
    return 0;
}

int CDraw::PrepareDrawPic()
{
    LOG_DEBUG<<"Enter the function CDraw::PrepareDrawPic()";

    glfwMakeContextCurrent(m_pGlfWindow);

    //int FWidth,FHeight;
    //glfwGetFramebufferSize(m_pGlfWindow,&FWidth,&FHeight);

    glewExperimental = GL_TRUE;
    char info[128] = {0};
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        sprintf(info, "GLEW Error: %s\n", glewGetErrorString(err));
        LOG_ERROR<<"glew init failed,error info is "<<info;
        return -1;
    }

    if (!m_strBG.empty())
    {
        m_uBGTextureID = SOIL_load_OGL_texture(m_strBG.c_str(),
                                              SOIL_LOAD_AUTO,
                                              SOIL_CREATE_NEW_ID,
                                              SOIL_FLAG_POWER_OF_TWO| SOIL_FLAG_MIPMAPS| SOIL_FLAG_DDS_LOAD_DIRECT);
    }


    if (!m_shaderManager.InitializeStockShaders())
    {
        LOG_ERROR<<"Init stock shaders failed";
        return -1;
    }

    glGenTextures(16,Textures);
    /*glViewport(0,0,FWidth,FHeight);*/
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(5);

    m_RenderText.Load("./fonts/simkai.ttf",18,18);

    LOG_DEBUG<<"Exit the function CDraw::PrepareDrawPic()";
    return 0;
}

int CDraw::DrawPic()
{
    if (!m_mapWindowsDatas.empty())
    {
        GLuint ID = m_shaderManager.UseStockShader(GLT_SHADER_TEXTURE_D,0,0);

        std::map<unsigned int,SubScreenPtr>::iterator it = m_mapWindowsDatas.begin();
        for (; it!=m_mapWindowsDatas.end(); /*++it*/)
        {
            if (!it->second->bvalid)
            {//当前子窗口被删掉了
                DeleteSubScreen(it->second);
                //it = m_mapWindowsDatas.erase(it);
                m_mapWindowsDatas.erase(it++);
            }
            else
            {
                SubScreenPtr pSubScreenData = it->second;
                ++it;
                
                GLuint uNextIndex = 0;
                //int iDataSize = 0;
				bool bNewFrame = false;
                SVideoData* pImageData = NULL;
                DataPtr pData;
				if (!pSubScreenData->bBGfalg)
                {
                    pSubScreenData->pDatas.GetData(bNewFrame,pData);
                    pImageData = (SVideoData*)(pData.get());

					pSubScreenData->uPBOIndex = (pSubScreenData->uPBOIndex + 1) % 2;
					uNextIndex = (pSubScreenData->uPBOIndex + 1) % 2;
                }

                if ((m_bFullSubScreen) && (m_iFullSubScreenID == pSubScreenData->ID))
                {
                    if (!m_bAlreadySetSize)
                    {
                        unsigned int uWidth = m_pMonitorMode->width;
                        unsigned int uHeight = m_pMonitorMode->height;
#ifdef WIN32
                        ::SetParent(glfwGetWin32Window(m_pGlfWindow),NULL);
#endif
                        glfwSetWindowSize(m_pGlfWindow,uWidth,uHeight);
                        m_bAlreadySetSize = true;
                    }

                    if (pSubScreenData->bBGfalg)
                    {//贴背景
                        if (m_uBGTextureID > 0)
                        {
							M3DMatrix44f mScreenSpace;
							m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_pMonitorMode->width, 0.0f, m_pMonitorMode->height, -1.0f, 1.0f);
							glUniformMatrix4fv(glGetUniformLocation(ID, "mvpMatrix"), 1, GL_FALSE, mScreenSpace);

							glViewport(0,0,m_pMonitorMode->width,m_pMonitorMode->height);
							glBindTexture(GL_TEXTURE_2D,m_uBGTextureID);
							pSubScreenData->pFullPic->Draw(ID);
							glBindTexture(GL_TEXTURE_2D,0);
                        }
                    }
                    else
                    {//播视频
						M3DMatrix44f mScreenSpace;
						m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_pMonitorMode->width, 0.0f, m_pMonitorMode->height, -1.0f, 1.0f);
						glUniformMatrix4fv(glGetUniformLocation(ID, "mvpMatrix"), 1, GL_FALSE, mScreenSpace);

                        glBindTexture(GL_TEXTURE_2D,pSubScreenData->TextureID);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[pSubScreenData->uPBOIndex]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pImageData->iWidth, pImageData->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

                        glViewport(0,0,m_pMonitorMode->width,m_pMonitorMode->height);

                        pSubScreenData->pFullPic->Draw(ID);

                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[uNextIndex]);
                        UpdateData(pImageData->pData.get(),pImageData->iWidth,pImageData->iHeight,pImageData->iWidth*pImageData->iHeight*3);

                        glBindTexture(GL_TEXTURE_2D,0);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

                        if (bNewFrame)
                        {
                            //计算分屏的帧率
                            pSubScreenData->GetFrameRate();
                        }

                        if (pSubScreenData->bDisplayText)
                        {

                            if (pSubScreenData->bTextMove)
                            {
                                if (enMovePolicy_Horizontal == pSubScreenData->MovePolicy)
                                {//水平移动
                                    if (pSubScreenData->bGetTextWidthflag)
                                    {
                                        m_RenderText.GetTextSumWidth(pSubScreenData->pText,pSubScreenData->ScaleSize,
																										pSubScreenData->fTextSumWidth);
                                    }

                                    pSubScreenData->bGetTextWidthflag = false;

									if (bNewFrame)
									{
										GLfloat fmove = pSubScreenData->SubSrceenPos.width / 80;
										pSubScreenData->fXMove -= fmove;
									}

                                    GLfloat fEnd = (-1.0f*(pSubScreenData->fTextSumWidth + 100));
                                    if (pSubScreenData->fXMove <= fEnd)
                                    {
                                        pSubScreenData->fXMove = pSubScreenData->SubSrceenPos.width;

                                    }

                                    if (((GLfloat)pSubScreenData->SubSrceenPos.width) != pSubScreenData->fXMove)
                                    {
                                        m_RenderText.RenderText(ID,
                                                                pSubScreenData->pText,
                                                                pSubScreenData->fXMove, 
                                                                pSubScreenData->yPos, 
                                                                pSubScreenData->ScaleSize,
                                                                glm::vec3(pSubScreenData->r, 
                                                                pSubScreenData->g, 
                                                                pSubScreenData->b));
                                    }

                                }
                                else if (enMovePolicy_vertical == pSubScreenData->MovePolicy)
                                {//垂直移动
                                    GLfloat y = 0.0f;
                                    if (0 == pSubScreenData->fYMove)
                                    {
                                        GLfloat fSumHeight = 0.0f;

                                        GLfloat fBeginMove = pSubScreenData->yPos - fSumHeight;
                                        pSubScreenData->fYMove = fBeginMove;
                                    }

                                    pSubScreenData->fYMove += pSubScreenData->fRate*5;

                                    if (pSubScreenData->fYMove > pSubScreenData->SubSrceenPos.height)
                                    {
                                        pSubScreenData->fYMove = 0;
                                    }

                                    if (0.0f != pSubScreenData->fYMove)
                                    {
                                        m_RenderText.RenderText(ID,
                                                                pSubScreenData->pText,
                                                                pSubScreenData->xPos,
                                                                pSubScreenData->fYMove,
                                                                pSubScreenData->ScaleSize,
                                                                glm::vec3(pSubScreenData->r, 
                                                                pSubScreenData->g, 
                                                                pSubScreenData->b));
                                    }

                                }
                            
                            }
                            else
                            {
                                m_RenderText.RenderText(ID,
                                                        pSubScreenData->pText,
                                                        pSubScreenData->xPos,
                                                        pSubScreenData->yPos,
                                                        pSubScreenData->ScaleSize,
                                                        glm::vec3(pSubScreenData->r, 
                                                        pSubScreenData->g, 
                                                        pSubScreenData->b));
                            }
                        
                        }
                    }
                }
                else if (!m_bFullSubScreen)
                {
                    if (m_bIsCancelFullScreen)
                    {//如果当前是取消全屏操作，则重新设置窗口大小
                        m_bAlreadySetSize = false;

                        if (NULL != m_pParent)
                        {
#ifdef WIN32
                            HWND pWnd = (HWND)(m_pParent);
                            HWND cWnd = glfwGetWin32Window(m_pGlfWindow);
                            ::SetParent(cWnd,pWnd);
#endif
                        }

                        glfwSetWindowSize(m_pGlfWindow,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
                        glfwSetWindowSize(m_pGlfWindow,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
						glViewport(0,0,m_VideoSrceenRect.width,m_VideoSrceenRect.height);

                        m_bIsCancelFullScreen = false;

                    }

                    if (pSubScreenData->bBGfalg)
                    {//贴背景
                        if (m_uBGTextureID > 0)
                        {
							M3DMatrix44f mScreenSpace;
							//m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_VideoSrceenRect.width, 0.0f, m_VideoSrceenRect.height, -1.0f, 1.0f);
							m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_VideoSrceenRect.width,-1.0f*m_VideoSrceenRect.height,0.0f,-1.0f,1.0f);
							glUniformMatrix4fv(glGetUniformLocation(ID, "mvpMatrix"), 1, GL_FALSE, mScreenSpace);

                            glBindTexture(GL_TEXTURE_2D,m_uBGTextureID);
                            pSubScreenData->pPic->Draw(ID);
                            glBindTexture(GL_TEXTURE_2D,0);
                        }

                        if (pSubScreenData->bSelect)
                        {
                            pSubScreenData->Selected(ID,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
                        }
                    
                    }
                    else
                    {//播视频

                        if (m_bIsCancelFullScreen)
                        {//如果当前是取消全屏操作，则重新设置窗口大小
                            m_bAlreadySetSize = false;

                            if (NULL != m_pParent)
                            {
    #ifdef WIN32
                                HWND pWnd = (HWND)(m_pParent);
                                HWND cWnd = glfwGetWin32Window(m_pGlfWindow);
                                ::SetParent(cWnd,pWnd);
    #endif
                            }

                             glfwSetWindowSize(m_pGlfWindow,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
                             glfwSetWindowSize(m_pGlfWindow,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
                        
                             m_bIsCancelFullScreen = false;
                        
                        }

						glViewport(0,0,m_VideoSrceenRect.width,m_VideoSrceenRect.height);

						M3DMatrix44f mScreenSpace;
						//m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_VideoSrceenRect.width, 0.0f, m_VideoSrceenRect.height, -1.0f, 1.0f);
						m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, m_VideoSrceenRect.width,-1.0f*m_VideoSrceenRect.height,0.0f,-1.0f,1.0f);
						glUniformMatrix4fv(glGetUniformLocation(ID, "mvpMatrix"), 1, GL_FALSE, mScreenSpace);

                        glBindTexture(GL_TEXTURE_2D,pSubScreenData->TextureID);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[pSubScreenData->uPBOIndex]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pImageData->iWidth, pImageData->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

                        pSubScreenData->pPic->Draw(ID);

                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[uNextIndex]);
                        UpdateData(pImageData->pData.get(),
												pImageData->iWidth,
												pImageData->iHeight,
												pImageData->iWidth*pImageData->iHeight*3);

                        glBindTexture(GL_TEXTURE_2D,0);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

                        if (bNewFrame)
                        {
                            //计算分屏的帧率
                            pSubScreenData->GetFrameRate();
                        }
                
                        if (pSubScreenData->bSelect)
                        {
                            pSubScreenData->Selected(ID,m_VideoSrceenRect.width,m_VideoSrceenRect.height);
                        }

                        if ((pSubScreenData->bDisplayText))
                        {
                             glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(pSubScreenData->SubSrceenPos.width), 0.0f,-1.0f * static_cast<GLfloat>(pSubScreenData->SubSrceenPos.height));
                             glUniformMatrix4fv(glGetUniformLocation(ID, "projectiont"), 1, GL_FALSE, glm::value_ptr(projection));

                            if ((pSubScreenData->bTextMove))
                            {
                                if (enMovePolicy_Horizontal == pSubScreenData->MovePolicy)
                                {//水平移动
                                    if (pSubScreenData->bGetTextWidthflag)
                                    {
                                        m_RenderText.GetTextSumWidth(pSubScreenData->pText,pSubScreenData->ScaleSize,pSubScreenData->fTextSumWidth);
                                    }

                                    pSubScreenData->bGetTextWidthflag = false;

									LOG_DEBUG<<"subscreen id "<<pSubScreenData->ID<<",fxmove "<<pSubScreenData->fXMove;
									
									if (bNewFrame)
									{
										GLfloat fmove = pSubScreenData->SubSrceenPos.width / 80;
										pSubScreenData->fXMove -= fmove;
									}

                                    GLfloat fEnd = (-1.0f*(pSubScreenData->fTextSumWidth + 100));
                                    if (pSubScreenData->fXMove <= fEnd)
                                    {
                                        pSubScreenData->fXMove = pSubScreenData->SubSrceenPos.width;
                                        
                                    }

                                    if (((GLfloat)pSubScreenData->SubSrceenPos.width) != pSubScreenData->fXMove)
                                    {
                                        m_RenderText.RenderText(ID,
                                                                pSubScreenData->pText,
                                                                pSubScreenData->fXMove, 
                                                                pSubScreenData->yPos, 
                                                                pSubScreenData->ScaleSize,
                                                                glm::vec3(pSubScreenData->r, 
                                                                pSubScreenData->g, 
                                                                pSubScreenData->b));
                                   }

                                }
                                else if (enMovePolicy_vertical == pSubScreenData->MovePolicy)
                                {//垂直移动
                                    GLfloat y = 0.0f;
                                    if (0 == pSubScreenData->fYMove)
                                    {
                                        GLfloat fSumHeight = 0.0f;
                                        GLfloat fBeginMove = pSubScreenData->yPos - fSumHeight;
                                        pSubScreenData->fYMove = fBeginMove;
                                    }

                                    pSubScreenData->fYMove += pSubScreenData->fRate*10;

                                    if (pSubScreenData->fYMove > pSubScreenData->SubSrceenPos.height)
                                    {
                                        pSubScreenData->fYMove = 0.0f;
                                    }

                                    if (0.0f != pSubScreenData->fYMove)
                                    {
                                        m_RenderText.RenderText(ID,
                                            pSubScreenData->pText,
                                            pSubScreenData->xPos,
                                            pSubScreenData->fYMove,
                                            pSubScreenData->ScaleSize,
                                            glm::vec3(pSubScreenData->r, 
                                            pSubScreenData->g, 
                                            pSubScreenData->b));
                                    }

                                }
                            }
                            else
                            {
                                m_RenderText.RenderText(ID,
                                                        pSubScreenData->pText,
                                                        pSubScreenData->xPos,
                                                        pSubScreenData->yPos,
                                                        pSubScreenData->ScaleSize,
                                                        glm::vec3(pSubScreenData->r, 
                                                        pSubScreenData->g, 
                                                        pSubScreenData->b));
                            }
                        }
                    }
                }
                
            }
         }
    }

    return 0;
}


int CDraw::SetWindowSize(int iWidth,int iHeight)
{
    if (NULL == m_pGlfWindow)
    {
        return -1;
    }

    glfwSetWindowSize(m_pGlfWindow,iWidth,iHeight);
    return 0;
}

int CDraw::SetWindowPos(int x,int y)
{
    if (NULL == m_pGlfWindow)
    {
        return -1;
    }

    glfwSetWindowPos(m_pGlfWindow,x,y);
    return 0;
}

bool CDraw::ChangeToFullDraw()
{
    if (-1 == m_iFullSubScreenID)
    {
        return false;
    }

    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(m_iFullSubScreenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return false;
    }

    if (NULL == m_pFullWindows)
    {
        m_pFullWindows = glfwCreateWindow(m_pMonitorMode->width,m_pMonitorMode->height,"",m_pMonitor,NULL);
        glfwMakeContextCurrent(m_pFullWindows);
        //PrepareDrawPic();
    }

    //unsigned int uWidth = m_pMonitorMode->width;
    //unsigned int uHeight = m_pMonitorMode->height;

    GLuint ID = m_shaderManager.UseStockShader(GLT_SHADER_TEXTURE_D,0,0);
    SubScreenPtr pSubScreenData = FindIt->second;
    
    GLuint uNextIndex = 0;
    pSubScreenData->uPBOIndex = (pSubScreenData->uPBOIndex + 1) % 2;
    uNextIndex = (pSubScreenData->uPBOIndex + 1) % 2;

    //int iDataSize = 0
	bool bNewFrame = false;
    DataPtr pData;
    pSubScreenData->pDatas.GetData(bNewFrame,pData);
    SVideoData* pImageData = (SVideoData*)(pData.get());

    glBindTexture(GL_TEXTURE_2D,pSubScreenData->TextureID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[pSubScreenData->uPBOIndex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pImageData->iWidth, pImageData->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
                    
                    
    glViewport(0,
               0,
               m_pMonitorMode->width,
               m_pMonitorMode->height);

    pSubScreenData->pPic->Draw(ID);

                    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pSubScreenData->uPBOs[uNextIndex]);
    UpdateData(pImageData->pData.get(),pImageData->iWidth,pImageData->iHeight,
                pImageData->iWidth*pImageData->iHeight*3);
                    
    glBindTexture(GL_TEXTURE_2D,0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

    if (bNewFrame)
    {
        //计算分屏的帧率
        pSubScreenData->GetFrameRate();
    }

    if (pSubScreenData->bDisplayText)
    {
        glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(pSubScreenData->SubSrceenPos.width), 0.0f, static_cast<GLfloat>(pSubScreenData->SubSrceenPos.height));
        glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        m_RenderText.RenderText(ID,
                                pSubScreenData->pText,
                                pSubScreenData->xPos, 
                                pSubScreenData->yPos,
                                pSubScreenData->ScaleSize,
                                glm::vec3(pSubScreenData->r, 
                                pSubScreenData->g, 
                                pSubScreenData->b));
    }

    return true;
}

int CDraw::SetTextMoveInfo(int SubScreenID,enMovePolicy enPolicy,float fRate)
{
    std::map<unsigned int,SubScreenPtr>::iterator FindIt = m_mapWindowsDatas.find(SubScreenID);
    if (FindIt == m_mapWindowsDatas.end())
    {
        return -1;
    }

    FindIt->second->bTextMove = true;
    FindIt->second->MovePolicy = enPolicy;
    FindIt->second->fRate = fRate;

    return 0;
}

void CDraw::UpdateData(unsigned char* pData,unsigned int iWidth,unsigned int Height,unsigned int iSize)
{
    glBufferData(GL_PIXEL_UNPACK_BUFFER,iSize,NULL,GL_DYNAMIC_READ);
    GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
    if (ptr)
    {
        /*SVideoData *pImageData = it->second.pDatas.front();*/
        /*if (fread(buf, 1, pixel_w*pixel_h*3/2, pFile[fileIndex]) != pixel_w*pixel_h*3/2)
        {
            // Loop
            fseek(pFile[fileIndex], 0, SEEK_SET);
            fread(buf, 1, pixel_w*pixel_h*3/2, pFile[fileIndex]);
        }*/

        //CONVERT_YUV420PtoRGB24(pData,ptr,iWidth,Height);
        memcpy(ptr,pData,iSize);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

        //释放图像数据
        //delete []ImageData.pData;
        //ImageData.pData = NULL;
    }
}

int CDraw::DeleteSubScreen(const SubScreenPtr &pData)
{
    glBindTexture(GL_TEXTURE_2D,0);
    glDeleteTextures(1,&(pData->TextureID));
    glDeleteBuffers(2,pData->uPBOs);

    //清空当前队列中的所有数据
    //pData->pDatas.Clear();

    return 0;
}

void CDraw::ChangeSizeFun(GLFWwindow* pWindow,int iWidth,int iHeight)
{
    CDraw *pData = (CDraw*)glfwGetWindowUserPointer(pWindow);
}

void CDraw::ErrorInfo(int errorno ,const char* errorinfo)
{
    std::string strErrorInfo = errorinfo;
    LOG_ERROR<<"glfw lib error info "<<errorinfo;
}

void CDraw::MouseButFun(GLFWwindow* pWindow,int button,int action,int mods)
{
    if (NULL != m_funMouseBtn)
    {
        m_funMouseBtn(ENMouseBtn(button),(ENMouseBtnAction)action);
    }
}

void CDraw::CursorposFun(GLFWwindow* pWindow,double x,double y)
{
    if (NULL != m_funCursorPos)
    {
        m_funCursorPos(x,y);
    }

}

void CDraw::SetMouseBtnCallback(MouseButtonCallBack f)
{
    m_funMouseBtn = f;
    glfwSetMouseButtonCallback(m_pGlfWindow,MouseButFun);
}

void CDraw::SetCursorCallback(CursorPosCallBack f)
{
    m_funCursorPos = f;
    glfwSetCursorPosCallback(m_pGlfWindow,CursorposFun);
}

int CDraw::GetSelectSubScreen()
{
    if (m_bFullSubScreen)
    {//如果当前是全屏状态，则直接返回当前是全屏的subscreen id
        return m_iFullSubScreenID;
    }

    double xPos = 0;
    double yPos = 0;
    glfwGetCursorPos(m_pGlfWindow,&xPos,&yPos);
    //屏幕的坐标系是左上角为远点，视口坐标系是左下角为原点
    double VxPos = xPos;
    double VyPos = -1* yPos;
    int SubScreenID = -1;
    std::map<unsigned int,SubScreenPtr>::iterator it = m_mapWindowsDatas.begin();
    for (; it != m_mapWindowsDatas.end(); ++it)
    {
        double sX = (double)it->second->SubSrceenPos.x;
        double sY = (double)it->second->SubSrceenPos.y;
        double xW = (double)it->second->SubSrceenPos.x + it->second->SubSrceenPos.width;
        double yH = (double)it->second->SubSrceenPos.y + it->second->SubSrceenPos.height;

        if (((VxPos >= sX) && (VxPos <= xW)) &&((VyPos <= sY) && (VyPos >= yH)))
        {
            SubScreenID = it->second->ID;
            break;
        }

    }

    return SubScreenID;
}

void CDraw::SelectSubScreen(int iSubScreenID)
{
    std::map<unsigned int,SubScreenPtr>::iterator it = m_mapWindowsDatas.begin();
    for (; it != m_mapWindowsDatas.end(); ++it)
    {
        if (it->second->ID == iSubScreenID)
        {
            it->second->bSelect = true;
        }
        else
        {
            it->second->bSelect = false;
        }
        
    }
}

void CDraw::SetFullScreen(int iSubScreenID)
{
    m_iFullSubScreenID = iSubScreenID;
    m_bFullSubScreen = true;
}

void CDraw::CancelFullScreen()
{
    m_iFullSubScreenID = -1;
    m_bFullSubScreen = false;
    m_bIsCancelFullScreen = true;
}