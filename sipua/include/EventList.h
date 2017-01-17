/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : EventList.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/11/07
* Description  : 事件链表程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __EVENT_LIST_H__
#define __EVENT_LIST_H__

#include "ua_port.h"
#include "LIST.H"

typedef struct event_data {
	char				*name;
	char				*value;
	struct event_data	*next;
} event_data_t;

typedef struct custom_event {	// 定制事件
	char			*subclass;	// 子类型
	event_data_t	*data;		// 值
} custom_event_t;

class CEventList
{
public:
	CEventList();
	virtual ~CEventList();

	// 以下函数可用于外部调用
	void set_list_max(int event_list_max);											// 事件缓冲大小(超过此值没处理会覆盖)
	custom_event_t *get_event(void);												// 获取事件

private:
	void destroy_event_list(void);													// 销毁事件链表

public:
	// 以下函数仅限于父类调用
	void set_semaphore(ua_sem_t *p_sem);											// 事件信号量
	void destroy_event(custom_event_t *p_event);									// 销毁事件
	custom_event_t *create_event(const char *p_subclass);							// 创建事件(返回值需要外部释放)
	BOOL event_add_data(custom_event_t *p_event, const char *p_name, const char *p_value);// 添加事件数据(name不能为空, value可以为空)
	BOOL add_event(custom_event_t *p_event);										// 添加一个事件(结构体)到链表(注: 会复制事件, p_event如果是动态分配, 须在外部释放)
	BOOL add_event(const char *p_subclass, const char *p_name, const char *p_value);// 添加一个事件(数据)到链表

public:

private:
    List<custom_event_t *>	m_list_event;							// 事件链表
    ua_mutex_t				m_mutex_list;							// m_list_event的互斥锁
	custom_event_t			*m_pcustom_event_get;					// 事件(用于查询返回)
	ua_sem_t				*m_psem_event;							// 事件信号量
	int						m_event_list_max;						// m_list_event最大数目
};

// 事件信号量
inline void CEventList::set_semaphore(ua_sem_t *p_sem)
{
	m_psem_event = p_sem;
}

// 事件缓冲大小(超过此值没处理会覆盖)
inline void CEventList::set_list_max(int event_list_max)
{
	m_event_list_max = event_list_max;
}

#endif // __EVENT_LIST_H__
