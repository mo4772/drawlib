#ifdef WIN32
#include <Windows.h>
#endif

#include "rendertextmanager.h"
#include "gltool/gltools.h"
#include "glfw3.h"
#include "glfw3native.h"
#include "baselib/log.h"

static GLfloat VC[]={0.0, 0.0,
                    0.0, 1.0,
                    1.0, 1.0,
                    0.0, 0.0,
                    1.0, 1.0,
                    1.0, 0.0};

CRenderTextManger::CRenderTextManger()
{}

CRenderTextManger::~CRenderTextManger()
{
    FT_Done_Face(m_FTFace);
    FT_Done_FreeType(m_FTLib);
}

int CRenderTextManger::Load(const char* FontFile,int W,int H)
{
    //初始化freetype库
    FT_Library library;
    if (FT_Init_FreeType(&library))
    {
        LOG_ERROR<<"Iint freetype lib failed";
        return -1;
    }

    //加载一个字体文件
    if (FT_New_Face(library,FontFile,0,&m_FTFace))
    {
        LOG_ERROR<<"Load fontfile "<<FontFile<<" failed";
        return -1;
    }

#ifdef WIN32
    FT_Select_Charmap(m_FTFace,FT_ENCODING_UNICODE);
#endif

    FT_Set_Pixel_Sizes(m_FTFace,0,48);

    //预加载生成多个字的纹理
    /*WCHar UnicodeString[]= L"武汉新图兴科电子股份有限公司中华人民共和国监控指挥调度空军海军陆军"
                        L"师连排班海康大华";*/
// #ifdef WIN32
//     WCHar *wstr = AnsiToUnicode(UnicodeString);
//     size_t uLen = wcslen(wstr);
//     for (int i=0; i<uLen; ++i)
//     {
//         SCharTexture tmp;
//         LoadCharTexture(wstr[i],tmp);
//     }
// #endif

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c=0; c<128; ++c)
    {//生成ASCII的纹理
        SCharTexture tmp;
        LoadCharTexture(c,tmp);
    }

    glGenVertexArrays(1,&m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 2, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    glGenBuffers(1,&m_TVBO);
    glBindBuffer(GL_ARRAY_BUFFER,m_TVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*6*2,VC,GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE, 0,0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    return 0;
}

int CRenderTextManger::LoadCharTexture(WCHar ch,SCharTexture& chartexture)
{
    std::map<WCHar,SCharTexture>::iterator it = m_mapCharTextures.find(ch);
    if (it == m_mapCharTextures.end())
    {//如果不存在该字体则生成对应的纹理
        if (FT_Load_Char(m_FTFace,ch,FT_LOAD_RENDER))
        {
            LOG_ERROR<<"FT_Load_Char return failed";
            return -1;
        }

        int a = 1,b = 1;
        if (49 == ch)
        {
            a = a+b;
        }

        GLuint texture;
        glGenTextures(1,&texture);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    m_FTFace->glyph->bitmap.width,
                    m_FTFace->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    m_FTFace->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        chartexture.TextureID = texture;
        chartexture.Size = glm::ivec2(m_FTFace->glyph->bitmap.width, m_FTFace->glyph->bitmap.rows);
        chartexture.Bearing = glm::ivec2(m_FTFace->glyph->bitmap_left, m_FTFace->glyph->bitmap_top);
        chartexture.Advance = m_FTFace->glyph->advance.x;

        m_mapCharTextures.insert(std::make_pair(ch,chartexture));
        glBindTexture(GL_TEXTURE_2D,0);
    }
    else
    {
        chartexture = it->second;
    }

    return 0;
}

#ifdef WIN32
LPWSTR CRenderTextManger::AnsiToUnicode(LPCTSTR lpcstr)
{
    int iLen = MultiByteToWideChar(CP_UTF8,0,lpcstr,-1,NULL,0);
    LPWSTR Pwstr = new WCHAR[iLen];
    MultiByteToWideChar(CP_UTF8,0,lpcstr,-1,Pwstr,iLen);

    return Pwstr;
}
#endif


void CRenderTextManger::SetMoveLimit(unsigned int uMin,unsigned int uMax)
{
    m_iMax = uMax;
    m_iMin = uMin;
}

void CRenderTextManger::GetTextSumWidth(const WCHar *strText,GLfloat scale,GLfloat &width)
{

#ifdef WIN32
    size_t uLen = wcslen(strText);
    for (int i=0; i<uLen; ++i)
    {
        SCharTexture tmp;
        LoadCharTexture(strText[i],tmp);
        GLfloat fPreX = tmp.Size.x * scale;
        width += fPreX;
    }
#endif

    //return fSumWidth;
}

// GLfloat CRenderTextManger::GetTextSumHeight(const char *strText,GLfloat scale)
// {
//     GLfloat fSumHeight = 0;
// 
// #ifdef WIN32
//     WCHar *wstr = AnsiToUnicode(strText);
//     size_t uLen = wcslen(wstr);
//     for (int i=0; i<uLen; ++i)
//     {
//         SCharTexture tmp;
//         LoadCharTexture(wstr[i],tmp);
//         GLfloat fPreX = tmp.Size.y * scale;
//         fSumHeight += fPreX;
//     }
// #endif
// 
//     size_t uLen = wcslen(strText);
//     for (int i=0; i<uLen; ++i)
//     {
//         SCharTexture ch;
//         LoadCharTexture(strText[i],ch);
//         GLfloat fPreX = ch.Size.x * scale;
//         fSumHeight += fPreX;
//     }
// 
//     return fSumHeight;
// }

void CRenderTextManger::RenderText(GLuint shader,
                                const WCHar *strText,
                                GLfloat x,
                                GLfloat y,
                                GLfloat scale,
                                glm::vec3 color)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniform1i(glGetUniformLocation(shader, "vtextflag"),GL_TRUE);
    glUniform1i(glGetUniformLocation(shader, "ftextflag"),GL_TRUE);
    /*glActiveTexture(GL_TEXTURE0);*/
    glBindVertexArray(m_VAO);

#ifdef WIN32
    size_t uLen = wcslen(strText);
#else
     size_t uLen = strlen(strText);
#endif

    for (int i=0;i<uLen;++i)
    //for (int i=uLen -1 ; i>=0; --i)
    {
        SCharTexture ch;

        LoadCharTexture(strText[i],ch);
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y)*scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

//         GLfloat vertices[6][4] = {
//             { xpos,     ypos + h,   0.0, 0.0 },            
//             { xpos,     ypos,       0.0, 1.0 },
//             { xpos + w, ypos,       1.0, 1.0 },
// 
//             { xpos,     ypos + h,   0.0, 0.0 },
//             { xpos + w, ypos,       1.0, 1.0 },
//             { xpos + w, ypos + h,   1.0, 0.0 }           
//         };

        GLfloat vertices[6][2] = {
            { xpos,     ypos + h},            
            { xpos,     ypos},
            { xpos + w, ypos},

            { xpos,     ypos + h},
            { xpos + w, ypos},
            { xpos + w, ypos + h}           
        };

        glBindTexture(GL_TEXTURE_2D,ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER,m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //复原状态
    glUniform1i(glGetUniformLocation(shader, "vtextflag"),GL_FALSE);
    glUniform1i(glGetUniformLocation(shader, "ftextflag"),GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}