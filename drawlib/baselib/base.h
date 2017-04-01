
#ifndef _KSNETTV_BASE_H
#define _KSNETTV_BASE_H

#undef NEW
#undef DELETE
#undef DELETE_A

#define NEW(p, type)  \
            {  \
                p = new type;  \
            }
           
#define DELETE(p)    \
            {  \
                if (NULL != p)  \
                {  \
                    delete p;  \
                    p = NULL;  \
                }  \
            }

#define DELETE_A(p)    \
            {  \
                if (NULL != p)  \
                {  \
                    delete [] p;  \
                    p = NULL;  \
                }  \
            }
#endif

