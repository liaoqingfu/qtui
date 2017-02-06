/*
* �ļ�����: SqQueue.H
* ժ    Ҫ: ѭ��������ģ��ͷ�ļ��������ļ�.
* ��ǰ�汾: 1.0
* ��    ��: ���ַ�
* �������: 2008��7��7��
*/

#ifndef SQQUEUE_H
#define SQQUEUE_H

#include <stdio.h>
//#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include "types.h"

//�ܱ������ݵĽ����ΪSQQUEUE_MAX_SIZE,�����������,�����������ݽ��Ḳ����ǰ�������
#define SQQUEUE_MAX_SIZE 100	
#define SQQUEUE_BUF_MAX_SIZE (SQQUEUE_MAX_SIZE+1) //��һ�������Ϊ�˱�֤ǰ�󲻻��ص�

#define SQ_PRINT(x)     printf(x)

//////////////////////////////////////////////////////
// SqQueue
template <class T>
class SqQueue
{
public:
	//���캯��
	SqQueue();			// ��ͨ���캯��
	SqQueue(const SqQueue &srcSqQueue);	// �������캯��

	//����
	SqQueue &operator =(const SqQueue &srcSqQueue);// ��ֵ����

	//����ͷ,β��������,����ձ�ͷ��ز���Ԥ�������!!!
	const T &GetHead() const;
	const T &GetTail() const;

	void InsertElem(const T &newElem);	//��β������һ����㱣��newElem
	BOOL DeleteElem(T &elem);	//��ͷ��ɾ��һ�����
	void Clear();		//�������
	int GetLength() const;	//���ؽ��ĸ���
	BOOL IsEmpty() const;	//�Ƿ�Ϊ�ձ�

	DWORD	m_dwOverflow;
	DWORD	m_dwRecvData;

	//����
private:
	T *m_lpBase;	//ѭ������̬�ڴ�Ļ�ַ
	int m_nFront;	//��ǰһ�����λ��
	int m_nRear;	//���һ�����λ��
	
	//��������
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
