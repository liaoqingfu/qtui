/*
* 文件名称: SqQueue.H
* 摘    要: 循环队列类模板头文件及定义文件.
* 当前版本: 1.0
* 作    者: 潘林峰
* 完成日期: 2008年7月7日
*/

#ifndef SQQUEUE_H
#define SQQUEUE_H

#include <stdio.h>
//#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include "types.h"

//能保存数据的结点数为SQQUEUE_MAX_SIZE,如果队列已满,继续插入数据将会覆盖最前面的数据
#define SQQUEUE_MAX_SIZE 100	
#define SQQUEUE_BUF_MAX_SIZE (SQQUEUE_MAX_SIZE+1) //留一个结点是为了保证前后不会重叠

#define SQ_PRINT(x)     printf(x)

//////////////////////////////////////////////////////
// SqQueue
template <class T>
class SqQueue
{
public:
	//构造函数
	SqQueue();			// 普通构造函数
	SqQueue(const SqQueue &srcSqQueue);	// 拷贝构造函数

	//操作
	SqQueue &operator =(const SqQueue &srcSqQueue);// 赋值函数

	//返回头,尾结点的数据,如果空表就返回不可预测的数据!!!
	const T &GetHead() const;
	const T &GetTail() const;

	void InsertElem(const T &newElem);	//在尾部插入一个结点保存newElem
	BOOL DeleteElem(T &elem);	//从头部删除一个结点
	void Clear();		//清空链表
	int GetLength() const;	//返回结点的个数
	BOOL IsEmpty() const;	//是否为空表

	DWORD	m_dwOverflow;
	DWORD	m_dwRecvData;

	//属性
private:
	T *m_lpBase;	//循环链表动态内存的基址
	int m_nFront;	//最前一个结点位置
	int m_nRear;	//最后一个结点位置
	
	//析构函数
public:
	virtual ~SqQueue();	
};

////////////////////////////////////////////////////
// List inline function
template <class T>
inline const T &SqQueue<T>::GetHead() const
{
	return m_lpBase[m_nFront];
}

template<class T>
inline const T &SqQueue<T>::GetTail() const
{
	return m_lpBase[m_nRear];
}

template<class T>
inline void SqQueue<T>::Clear()
{
	m_nFront = m_nRear;
}

template<class T>
inline int SqQueue<T>::GetLength() const
{
	return (m_nRear >= m_nFront ? m_nRear : m_nRear + SQQUEUE_BUF_MAX_SIZE) - m_nFront;
}

template<class T>
inline BOOL SqQueue<T>::IsEmpty() const
{
	return GetLength() == 0 ? TRUE : FALSE;
}
////////////////////////////////////////////////////
// List out-of-line function
template<class T>
SqQueue<T>::SqQueue() : m_nFront(0), m_nRear(0)
{
	m_lpBase = (T *) new T[SQQUEUE_BUF_MAX_SIZE];
	if (m_lpBase == NULL)
	{
		SQ_PRINT("SqQueue : Allocate memory error!\n");
		exit(1);
	}
	m_dwRecvData = m_dwOverflow = 0;
}

template<class T>
SqQueue<T>::SqQueue(const SqQueue<T> &srcSqQueue)
{
	m_lpBase = (T *) new T[SQQUEUE_BUF_MAX_SIZE];
	if (m_lpBase == NULL)
	{
		SQ_PRINT("SqQueue : Allocate memory error!\n");
		exit(1);
	}
	*this = srcSqQueue;
	m_dwRecvData = m_dwOverflow = 0;
}

template<class T>
SqQueue<T> & SqQueue<T>::operator =(const SqQueue<T> &srcSqQueue)
{
	for (int i = 0; i < SQQUEUE_BUF_MAX_SIZE; i++)
	{
		memcpy(&m_lpBase[i], &(srcSqQueue.m_lpBase[i]), sizeof(T));
	}
	m_nFront = srcSqQueue.m_nFront;
	m_nRear = srcSqQueue.m_nRear;
	return *this;
}

template<class T>
void SqQueue<T>::InsertElem(const T &newElem)
{
	memcpy(&m_lpBase[m_nRear], &newElem, sizeof(T));
	m_nRear = (m_nRear + 1) % SQQUEUE_BUF_MAX_SIZE;
	if (m_nRear == m_nFront)
	{
		m_nFront = (m_nFront + 1) % SQQUEUE_BUF_MAX_SIZE;
		//SQ_PRINT("SqQueue : Queue overflow!\n");
		m_dwOverflow++;
	}
	m_dwRecvData++;
}

template<class T>
BOOL SqQueue<T>::DeleteElem(T &elem)
{
	BOOL bRet = FALSE;
	if (m_nFront != m_nRear)
	{
		memcpy(&elem, &m_lpBase[m_nFront], sizeof(T));
		m_nFront = (m_nFront + 1) % SQQUEUE_BUF_MAX_SIZE;
		bRet = TRUE;
	}
	
	return bRet;
}

template<class T>
SqQueue<T>::~SqQueue()
{
	delete m_lpBase;
	m_lpBase = NULL;
}

#endif //SQQUEUE_H
