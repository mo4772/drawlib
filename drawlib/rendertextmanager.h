#ifndef RENDER_TEXT_MANAGER_H
#define RENDER_TEXT_MANAGER_H

#include <map>
#include "GL/glew.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/fttrigon.h"
#include "typedefine.h"
#include "glm/glm.hpp"
struct SCharTexture
{
    GLuint TextureID;
    //字体的大小
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    //字之间的间距
    GLuint Advance;
};

class CRenderTextManger
{
    public:
        CRenderTextManger();
        ~CRenderTextManger();

        int Load(const char* FontFile,int W,int H);
        int LoadCharTexture(WCHar ch,SCharTexture& chartexture);
        //移动的范围
        void SetMoveLimit(unsigned int uMin,unsigned int uMax);

        void GetTextSumWidth(const WCHar *strText,GLfloat scale,GLfloat &width);
        /*GLfloat GetTextSumHeight(const char *strText,GLfloat scale);*/

        void RenderText(GLuint shader,
                        const WCHar *strText,
                        GLfloat x,
                        GLfloat y,
                        GLfloat scale,
                        glm::vec3 color);

#ifdef WIN32
        LPWSTR AnsiToUnicode(LPCTSTR lpcstr);
#endif

    private:
        FT_Library m_FTLib;
        FT_Face m_FTFace;
    private:
        std::map<WCHar,SCharTexture> m_mapCharTextures;
        GLuint m_VAO;
        GLuint m_VBO;
        GLuint m_TVBO;

        //最小值
        int m_iMin;
        //最大值
        int m_iMax;

};
#endif