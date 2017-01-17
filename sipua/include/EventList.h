/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : EventList.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/11/07
* Description  : �¼��������
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

typedef struct custom_event {	// �����¼�
	char			*subclass;	// ������
	event_data_t	*data;		// ֵ
} custom_event_t;

class CEventList
{
public:
	CEventList();
	virtual ~CEventList();

	// ���º����������ⲿ����
	void set_list_max(int event_list_max);											// �¼������С(������ֵû����Ḳ��)
	custom_event_t *get_event(void);												// ��ȡ�¼�

private:
	void destroy_event_list(void);													// �����¼�����

public:
	// ���º��������ڸ������
	void set_semaphore(ua_sem_t *p_sem);											// �¼��ź���
	void destroy_event(custom_event_t *p_event);									// �����¼�
	custom_event_t *create_event(const char *p_subclass);							// �����¼�(����ֵ��Ҫ�ⲿ�ͷ�)
	BOOL event_add_data(custom_event_t *p_event, const char *p_name, const char *p_value);// ����¼�����(name����Ϊ��, value����Ϊ��)
	BOOL add_event(custom_event_t *p_event);										// ���һ���¼�(�ṹ��)������(ע: �Ḵ���¼�, p_event����Ƕ�̬����, �����ⲿ�ͷ�)
	BOOL add_event(const char *p_subclass, const char *p_name, const char *p_value);// ���һ���¼�(����)������

public:

private:
    List<custom_event_t *>	m_list_event;							// �¼�����
    ua_mutex_t				m_mutex_list;							// m_list_event�Ļ�����
	custom_event_t			*m_pcustom_event_get;					// �¼�(���ڲ�ѯ����)
	ua_sem_t				*m_psem_event;							// �¼��ź���
	int						m_event_list_max;						// m_list_event�����Ŀ
};

// �¼��ź���
inline void CEventList::set_semaphore(ua_sem_t *p_sem)
{
	m_psem_event = p_sem;
}

// �¼������С(������ֵû����Ḳ��)
inline void CEventList::set_list_max(int event_list_max)
{
	m_event_list_max = event_list_max;
}

#endif // __EVENT_LIST_H__
