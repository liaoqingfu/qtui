/*
* �ļ�����: LIST.H
* ժ    Ҫ: ˫��������ģ��ͷ�ļ��������ļ�.
* ��ǰ�汾: 1.0
* ��    ��: ���ַ�
* �������: 2007��12��9��
*/

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include "types.h"
using namespace std; 

//////////////////////////////////////////////////////
// List
template<class T>
class List		//��ͷ����˫������(ͷ��㲻��������)
{			//����ע����'ͷ���'��ָ'��һ�����ݽ��'
protected:
	struct LNode
	{
		LNode *prior;		//ָ��ǰ�����
		LNode *next;		//ָ���̽��
		T data;
	};

public:
//���캯��
	List();			// ��ͨ���캯��
	List(const List &srcList);	// �������캯��

//����
	List &operator =(const List &srcList);// ��ֵ����

	int GetCount() const;	//���ؽ�����
	BOOL IsEmpty() const;	//�ж������Ƿ�Ϊ��

	// posΪ1��ʼ,���ص�pos����������,pos���Ϸ��򷵻ز���Ԥ�������!!!
	BOOL GetElem(int pos, T &elem);

	// posΪ1��ʼ,��elemȡ����pos����������
	BOOL SetElem(int pos, const T &elem);

	//����ͷ,β��������,����ձ��ͷ��ز���Ԥ�������!!!
	const T &GetHead() const;
	const T &GetTail() const;

	//��pos��ʼ����Ѱ�ҵ�һ����elem��ȵ����ݵ�λ��
	int Find(const T &elem, int pos = 0);

	void Init(void);
	BOOL RemoveHead(T &elem);	//��ͷ��ɾ��һ�����
	BOOL RemoveTail(T &elem);	//��β��ɾ��һ�����
	BOOL Remove(int pos, T &elem);	//ɾ����pos�����
	void AddHead(const T &newElem);	//��ͷ������һ����㱣��newElem
	void AddTail(const T &newElem);	//��β������һ����㱣��newElem
	void AddHead(const List &newList);	//��ͷ������һ������newList
	void AddTail(const List &newList);//��β������һ������newList
	BOOL InsertAfter(int pos, const T &newElem); //��pos�������һ�����
	BOOL InsertBefore(int pos, const T &newElem); //��posǰ�����һ�����
	void Clear();		//�������
	BOOL TraverseHead(BOOL (*visit)(T &elem)); //��ͷ��ÿ���������ݵ���visit
	BOOL TraverseTail(BOOL (*visit)(T &elem)); //��β��ÿ���������ݵ���visit

	//������������(���ָ���,��ű�)��������,ʱ�临�Ӷ�:n*n,�ռ临�Ӷ�:n*(ָ��+int)
	void Sort(); 		//ÿ����¼ֻ�ƶ�һ��, ģ����'T'�����в���������'>'

protected:
	void *CreateNode(const T &newElem);
	void *GetPos(int pos);	//���ص�pos������λ��,pos���Ϸ��򷵻�m_head

//����
private:
	LNode *m_head;	//ͷָ��
	LNode *m_tail;	//βָ��
	LNode *m_just;	//�û����һ�η��ʵĽ��λ��
	int m_justNum;	//�û����һ�η��ʵĽ�����(��ͷ��β���)
	int m_count;		//������(������ͷ���)

//��������
public:
	virtual ~List();	
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// List inline function
template<class T>
inline int List<T>::GetCount() const
{
	return m_count;
}

template<class T>
inline BOOL List<T>::IsEmpty() const
{
	return (BOOL)(m_count == 0);
}

template<class T>
inline BOOL List<T>::GetElem(int pos, T &elem)
{
	LNode *posNode = (LNode *)GetPos(pos);
	if (posNode == m_head)
		return FALSE;
	else
	{
		elem = posNode->data;
		return TRUE;
	}
}

template<class T>
inline BOOL List<T>::SetElem(int pos, const T &elem)
{
	LNode *posNode = (LNode *)GetPos(pos);
	if (posNode == m_head)
		return FALSE;
	else
	{
		posNode->data = elem;
		return TRUE;
	}
}

template<class T>
inline const T &List<T>::GetHead() const
{
	if (m_count != 0)
		return m_head->next->data;
	else
		return m_head->data;
}

template<class T>
inline const T &List<T>::GetTail() const
{
	return m_tail->data;
}

////////////////////////////////////////////////////
// List out-of-line function
template<class T>
List<T>::List() : m_head(new LNode), m_count(0), m_justNum(0)
{
	m_head->next = m_head->prior = NULL;
	m_just = m_tail = m_head;
}

template<class T>
void List<T>::Init(void)
{
	if (m_head == NULL)
	{
		m_count = 0;
		m_justNum = 0;
		m_head = new LNode;
		m_head->next = m_head->prior = NULL;
		m_head->data = NULL;
		m_just = m_tail = m_head;
	}
}

template<class T>
List<T>::List(const List<T> &srcList)
	:m_head(new LNode), m_count(0), m_justNum(0)
{
	m_head->next = m_head->prior = NULL;
	m_head->data = NULL;
	m_just = m_tail = m_head;
	this->AddTail(srcList);
}

template<class T>
List<T> & List<T>::operator =(const List<T> &srcList)
{
	LNode *destNode = m_head;
	LNode *srcNode = srcList.m_head;
	while (destNode != NULL && srcNode != NULL)
	{
		destNode->data = srcNode->data;
		destNode = destNode->next;
		srcNode = srcNode->next;
	}
	if (destNode == NULL)	//Դ������,Դ�������β��
	{
		while (srcNode != NULL)
		{
			this->AddTail(srcNode->data);
			srcNode = srcNode->next;
		}
	}
	else if (srcNode == NULL)	//Ŀ��������,ɾ��Ŀ������
	{
		destNode = destNode->prior;
		T temp;
		do
		{
			this->RemoveTail(temp);
		} while (destNode != m_tail);
	}
	m_count = srcList.m_count;
	m_just = m_head;
	m_justNum = 0;
	return *this;
}

template<class T>
int List<T>::Find(const T &elem, int pos)
{
	LNode *curNode = m_head->next;
	int curPos = 1;
	if (pos != 0)
	{
		curNode = (LNode *)GetPos(pos);
		if (curNode == m_head)	//pos���Ϸ�
			curPos = 0;
		else
			curPos = pos;
	}
	if (curPos != 0)
	{
		m_head->data = elem;		//�ڱ�
		m_tail->next = m_head;
		for (; curNode->data != elem;curNode = curNode->next,++curPos);
		m_tail->next = NULL;
		m_head->data = NULL;
	}
	if (curNode == m_head)
		curPos = 0;
	return curPos;
}

template<class T>
BOOL List<T>::RemoveHead(T &elem)
{
	if (m_count == 0)
		return FALSE;
	if (m_justNum != 0)	//�޸����һ�η��ʵĽ��λ�ü����
	{
		if (--m_justNum == 0)
			m_just = m_head;
	}
	elem = m_head->next->data;
	if (m_count == 1)
	{
		delete m_head->next;
		m_head->next = NULL;
		m_tail = m_head;
	}
	else
	{
		m_head->next = m_head->next->next;
		delete m_head->next->prior;
		m_head->next->prior = m_head;
	}
	--m_count;
	return TRUE;
}

template<class T>
BOOL List<T>::RemoveTail(T &elem)
{
	if (m_count == 0)
		return FALSE;
	if (m_justNum == m_count)	//�޸����һ�η��ʵĽ��λ�ü����
	{
		m_justNum = 0;
		m_just = m_head;
	}
	elem = m_tail->data;
	m_tail = m_tail->prior;
	delete m_tail->next;
	m_tail->next = NULL;
	--m_count;
	return TRUE;
}

template<class T>
BOOL List<T>::Remove(int pos, T &elem)
{
	LNode *posNode = (LNode *)GetPos(pos);
	if (posNode == m_head)	//pos���Ϸ�
		return FALSE;
	if (posNode == m_head->next)
		RemoveHead(elem);
	else if (posNode == m_tail)
		RemoveTail(elem);
	else
	{
		elem = posNode->data;
		m_justNum = pos -1;	//�޸����һ�η��ʵĽ��λ�ü����
		m_just = posNode->prior;
		posNode->next->prior = posNode->prior;
		posNode->prior->next = posNode->next;
		delete posNode;
		//m_just = m_head;
		--m_count;
	}
	return TRUE;
}

template<class T>
void List<T>::AddHead(const T &newElem)
{
	LNode *newNode = (LNode *)CreateNode(newElem);
	if (m_justNum != 0)	//�޸����һ�η��ʵĽ��λ�ü����
		++m_justNum;
	newNode->prior = m_head;
	newNode->next = m_head->next;
	m_head->next = newNode;
	if (newNode->next != NULL)
		newNode->next->prior = newNode;
	else
		m_tail = newNode;
	++m_count;
}

template<class T>
void List<T>::AddTail(const T &newElem)
{
	LNode *newNode = (LNode *)CreateNode(newElem);
	newNode->prior = m_tail;
	newNode->next = NULL;
	m_tail->next = newNode;
	m_tail = newNode;
	++m_count;
}

template<class T>
void List<T>::AddHead(const List<T> &newList)
{
	LNode *srcNode = newList.m_tail;
	while (srcNode != newList.m_head)
	{
		AddHead(srcNode->data);
		srcNode = srcNode->prior;
	}
}

template<class T>
void List<T>::AddTail(const List<T> &newList)
{
	LNode *srcNode = newList.m_head->next;
	while (srcNode != NULL)
	{
		AddTail(srcNode->data);
		srcNode = srcNode->next;
	}
}

template<class T>
BOOL List<T>::InsertAfter(int pos, const T &newElem)
{
	LNode *posNode = (LNode *)GetPos(pos);
	if (posNode == m_head)	//pos���Ϸ�
		return FALSE;

	if (posNode->next == NULL)		//posΪβ���
		AddTail(newElem);
	else
	{
		LNode *newNode = (LNode *)CreateNode(newElem);
		newNode->next = posNode->next;
		newNode->prior = posNode;
		newNode->next->prior = newNode;
		posNode->next = newNode;
		++m_count;
	}
	return TRUE;
}

template<class T>
BOOL List<T>::InsertBefore(int pos, const T &newElem)
{
	LNode *posNode = (LNode *)GetPos(pos);
	if (posNode == m_head)
		return FALSE;

	LNode *newNode = (LNode *)CreateNode(newElem);
	newNode->prior = posNode->prior;
	newNode->next = posNode;
	newNode->prior->next = newNode;
	posNode->prior = newNode;
	++m_count;
	++m_justNum;
	return TRUE;
}

template<class T>
void List<T>::Clear()
{
	LNode *curNode = m_head->next;
	LNode *nextNode = NULL;
	while (curNode != NULL)
	{
		nextNode = curNode->next;
		delete curNode;
		curNode = nextNode;
	}
	m_head->next = NULL;
	m_just = m_tail = m_head;
	m_justNum = m_count = 0;
}

template<class T>
BOOL List<T>::TraverseHead(BOOL (*visit)(T &elem))
{
	LNode *curNode = m_head->next;
	while (curNode != NULL)
	{
		if (!visit(curNode->data))
			return FALSE;
		curNode = curNode->next;
	}
	return TRUE;
}

template<class T>
BOOL List<T>::TraverseTail(BOOL (*visit)(T &elem))
{
	LNode *curNode = m_tail;
	while (curNode != m_head)
	{
		if (!visit(curNode->data))
			return FALSE;
		curNode = curNode->prior;
	}
	return TRUE;
}

template<class T>
void List<T>::Sort()
{
	if (m_count < 2)
		return;	

	LNode **cursor = new LNode *[m_count + 1];//�±�1ָ���1�����...
	int *pos = new int[m_count + 1];//next��,'0'ָ����С�Ľ��ָ��
	int i,j;
	
	cursor[0] = m_head;	//��ʼ��ָ����
	for (i=1; i<=m_count; i++)
		cursor[i] = cursor[i-1]->next;
	//��������
	pos[0] = 1;
	pos[1] = 0;	//1��0����ѭ������
	int priorPos;
	for (i=2; i<=m_count; i++)
	{
		priorPos = 0;
		j = pos[0];
		cursor[0]->data = cursor[i]->data;	//�ڱ�
		while (cursor[i]->data > cursor[j]->data)
		{
			priorPos = j;
			j = pos[j];
		}
		pos[priorPos] = i;
		pos[i] = j;
	}
	//���Ž��
	int &curPos = priorPos;
	LNode *curNode = cursor[curPos];
	LNode *priorNode = m_head;
	for (i=1; i<=m_count; i++)
	{
		priorNode->next = curNode;
		curNode->prior = priorNode;
		priorNode = curNode;
		curPos = pos[curPos];
		curNode = cursor[curPos];
	}
	priorNode->next = NULL;
	m_tail = priorNode;
	m_just = m_head;
	m_justNum = 0;
	
	delete [] cursor;
	delete [] pos;
}
	
template<class T>
void *List<T>::CreateNode(const T &newElem)
{
	LNode *newNode = new LNode;
	if (newNode == NULL)
	{
		printf("Memory Exhausted\n");
		return NULL;
	}
	newNode->data = newElem;
	return newNode;
}

template<class T>
void *List<T>::GetPos(int pos)
{
	if (pos < 1 || pos > m_count)
		return m_head;
	int direction = 0;		//�����־

	if (pos < m_justNum)		//pos��m_justNum��ǰ��
	{
		if ((m_justNum - pos) > pos) //pos����m_head
		{
			m_justNum = 1;
			m_just = m_head->next;
			direction = 1;
		}
		else		//pos����m_justNum
			direction = -1;
	}
	else if (pos > m_justNum)	//pos��m_justNum�ĺ���
	{
		if ((pos - m_justNum) > (m_count - pos)) //pos����m_tail
		{
			m_justNum = m_count;
			m_just = m_tail;
			direction = -1;
		}
		else		//pos����m_justNum
			direction = 1;
	}

	if (direction > 0)	//������
	{
		for (; m_justNum < pos; m_justNum++)
			m_just = m_just->next;
	}
	else if (direction < 0)	//��ǰ��
	{
		for (; m_justNum > pos; m_justNum--)
			m_just = m_just->prior;
	}
				
	return m_just;
}

template<class T>
List<T>::~List()
{
	this->Clear();
	delete m_head;
}

#endif