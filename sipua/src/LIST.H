/*
* 文件名称: LIST.H
* 摘    要: 双向链表类模板头文件及定义文件.
* 当前版本: 1.0
* 作    者: 潘林峰
* 完成日期: 2007年12月9日
*/

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include "types.h"
using namespace std; 

//////////////////////////////////////////////////////
// List
template<class T>
class List		//带头结点的双向链表(头结点不保存数据)
{			//以下注释里'头结点'均指'第一个数据结点'
protected:
	struct LNode
	{
		LNode *prior;		//指向前驱结点
		LNode *next;		//指向后继结点
		T data;
	};

public:
//构造函数
	List();			// 普通构造函数
	List(const List &srcList);	// 拷贝构造函数

//操作
	List &operator =(const List &srcList);// 赋值函数

	int GetCount() const;	//返回结点个数
	BOOL IsEmpty() const;	//判断链表是否为空

	// pos为1起始,返回第pos个结点的数据,pos不合法则返回不可预测的数据!!!
	BOOL GetElem(int pos, T &elem);

	// pos为1起始,用elem取代第pos个结点的数据
	BOOL SetElem(int pos, const T &elem);

	//返回头,尾结点的数据,如果空表就返回不可预测的数据!!!
	const T &GetHead() const;
	const T &GetTail() const;

	//从pos开始往后寻找第一个与elem相等的数据的位置
	int Find(const T &elem, int pos = 0);

	void Init(void);
	BOOL RemoveHead(T &elem);	//从头部删除一个结点
	BOOL RemoveTail(T &elem);	//从尾部删除一个结点
	BOOL Remove(int pos, T &elem);	//删除第pos个结点
	void AddHead(const T &newElem);	//在头部插入一个结点保存newElem
	void AddTail(const T &newElem);	//在尾部插入一个结点保存newElem
	void AddHead(const List &newList);	//在头部插入一个链表newList
	void AddTail(const List &newList);//在尾部插入一个链表newList
	BOOL InsertAfter(int pos, const T &newElem); //在pos后面插入一个结点
	BOOL InsertBefore(int pos, const T &newElem); //在pos前面插入一个结点
	void Clear();		//清空链表
	BOOL TraverseHead(BOOL (*visit)(T &elem)); //从头对每个结点的数据调用visit
	BOOL TraverseTail(BOOL (*visit)(T &elem)); //从尾对每个结点的数据调用visit

	//用两个辅助表(结点指针表,序号表)插入排序,时间复杂度:n*n,空间复杂度:n*(指针+int)
	void Sort(); 		//每个记录只移动一次, 模板类'T'必须有操作符重载'>'

protected:
	void *CreateNode(const T &newElem);
	void *GetPos(int pos);	//返回第pos个结点的位置,pos不合法则返回m_head

//属性
private:
	LNode *m_head;	//头指针
	LNode *m_tail;	//尾指针
	LNode *m_just;	//用户最后一次访问的结点位置
	int m_justNum;	//用户最后一次访问的结点序号(从头往尾编号)
	int m_count;		//结点个数(不包括头结点)

//析构函数
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
	if (destNode == NULL)	//源链表长,源余结点接在尾部
	{
		while (srcNode != NULL)
		{
			this->AddTail(srcNode->data);
			srcNode = srcNode->next;
		}
	}
	else if (srcNode == NULL)	//目标链表长,删除目标余结点
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
		if (curNode == m_head)	//pos不合法
			curPos = 0;
		else
			curPos = pos;
	}
	if (curPos != 0)
	{
		m_head->data = elem;		//哨兵
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
	if (m_justNum != 0)	//修改最后一次访问的结点位置及序号
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
	if (m_justNum == m_count)	//修改最后一次访问的结点位置及序号
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
	if (posNode == m_head)	//pos不合法
		return FALSE;
	if (posNode == m_head->next)
		RemoveHead(elem);
	else if (posNode == m_tail)
		RemoveTail(elem);
	else
	{
		elem = posNode->data;
		m_justNum = pos -1;	//修改最后一次访问的结点位置及序号
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
	if (m_justNum != 0)	//修改最后一次访问的结点位置及序号
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
	if (posNode == m_head)	//pos不合法
		return FALSE;

	if (posNode->next == NULL)		//pos为尾结点
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

	LNode **cursor = new LNode *[m_count + 1];//下标1指向第1个结点...
	int *pos = new int[m_count + 1];//next域,'0'指向最小的结点指针
	int i,j;
	
	cursor[0] = m_head;	//初始化指针域
	for (i=1; i<=m_count; i++)
		cursor[i] = cursor[i-1]->next;
	//插入排序
	pos[0] = 1;
	pos[1] = 0;	//1和0建立循环链表
	int priorPos;
	for (i=2; i<=m_count; i++)
	{
		priorPos = 0;
		j = pos[0];
		cursor[0]->data = cursor[i]->data;	//哨兵
		while (cursor[i]->data > cursor[j]->data)
		{
			priorPos = j;
			j = pos[j];
		}
		pos[priorPos] = i;
		pos[i] = j;
	}
	//重排结点
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
	int direction = 0;		//方向标志

	if (pos < m_justNum)		//pos在m_justNum的前面
	{
		if ((m_justNum - pos) > pos) //pos靠近m_head
		{
			m_justNum = 1;
			m_just = m_head->next;
			direction = 1;
		}
		else		//pos靠近m_justNum
			direction = -1;
	}
	else if (pos > m_justNum)	//pos在m_justNum的后面
	{
		if ((pos - m_justNum) > (m_count - pos)) //pos靠近m_tail
		{
			m_justNum = m_count;
			m_just = m_tail;
			direction = -1;
		}
		else		//pos靠近m_justNum
			direction = 1;
	}

	if (direction > 0)	//往后找
	{
		for (; m_justNum < pos; m_justNum++)
			m_just = m_just->next;
	}
	else if (direction < 0)	//往前找
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
