#ifndef PC_DEUQUE_H
#define PC_DEUQUE_H
#include <deque>
#include "boost/smart_ptr.hpp"
#include "tinycthread.h"

#define MAX_CNT 60

typedef boost::shared_ptr<void> DataPtr;
class CPCDeque 
{   
    public:
        CPCDeque();
        ~CPCDeque();

        void PutData(const DataPtr &pData);
        //void* Front();
        //void PopFront();

        bool IsEmpty();
        void Clear();

        void GetData(bool &newFrame,DataPtr &pData);

        unsigned int Size();

    private:
        std::deque<DataPtr> m_deque;
        
        cnd_t m_CndEmpty;
        mtx_t m_mutexEmpty;

        cnd_t m_CndFull;
        mtx_t m_mutexFull;
};

#endif