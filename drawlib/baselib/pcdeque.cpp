#include "pcdeque.h"
#include <iostream>

CPCDeque::CPCDeque()
{
    mtx_init(&m_mutexEmpty,mtx_plain);
    cnd_init(&m_CndEmpty);

    mtx_init(&m_mutexFull,mtx_plain);
    cnd_init(&m_CndFull);
}

CPCDeque::~CPCDeque()
{
    mtx_destroy(&m_mutexEmpty);
    cnd_destroy(&m_CndEmpty);

    mtx_destroy(&m_mutexFull);
    cnd_destroy(&m_CndFull);
}

void CPCDeque::PutData(const DataPtr &pData)
{//生产端

    bool bNew = false;
    mtx_lock(&m_mutexEmpty);
    while (m_deque.size() >= MAX_CNT)
    {//队列已满
        cnd_wait(&m_CndEmpty,&m_mutexEmpty);
    }

    mtx_unlock(&m_mutexEmpty);

    mtx_lock(&m_mutexFull);
    m_deque.push_back(pData);
    bNew = true;
    mtx_unlock(&m_mutexFull);

    if (bNew)
    {
        cnd_signal(&m_CndFull);
    }
}

/*void* CPCDeque::Front()
{
    void* pData = NULL;

    mtx_lock(&m_mutexFull);
    while (m_deque.empty())
    {
        cnd_wait(&m_CndFull,&m_mutexFull);
    }

    pData = m_deque.front();
    mtx_unlock(&m_mutexFull);

    return pData;
}

void CPCDeque::PopFront()
{
    mtx_lock(&m_mutexFull);
    m_deque.pop_front();
    mtx_unlock(&m_mutexFull);

    cnd_signal(&m_CndEmpty);
}*/

bool CPCDeque::IsEmpty()
{
    bool bEmpty = false;

    mtx_lock(&m_mutexFull);
    bEmpty = m_deque.empty();
    mtx_unlock(&m_mutexFull);

    return bEmpty;
}

void CPCDeque::Clear()
{
    mtx_lock(&m_mutexFull);
    m_deque.clear();
    mtx_unlock(&m_mutexFull);

    cnd_signal(&m_CndEmpty);
}

unsigned int CPCDeque::Size()
{
    int iSize = 0;
    
    mtx_lock(&m_mutexFull);
    iSize = m_deque.size();
    mtx_unlock(&m_mutexFull);
    
    return iSize;
}

void CPCDeque::GetData(bool &newFrame,DataPtr& pData)
{
    mtx_lock(&m_mutexFull);
    if (m_deque.empty())
    {
        cnd_wait(&m_CndFull,&m_mutexFull);
    }

    pData = m_deque.front();
    if ( m_deque.size() > 1)
    {
		newFrame = true;
        m_deque.pop_front();
    }

    mtx_unlock(&m_mutexFull);

    cnd_signal(&m_CndEmpty);
}

