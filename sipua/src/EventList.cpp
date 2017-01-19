/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : EventList.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/11/07
* Description  : �¼��������
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#include "EventList.h"
#include "ua_global.h"

CEventList::CEventList()
{
	ua_mutex_init(&m_mutex_list);
	m_pcustom_event_get = NULL;
	m_psem_event = NULL;
	m_event_list_max = 100;
}

CEventList::~CEventList()
{
	m_psem_event = NULL;
	destroy_event_list();
	ua_mutex_destroy(&m_mutex_list);
	if (m_pcustom_event_get != NULL)
	{
		destroy_event(m_pcustom_event_get);
		ua_free(m_pcustom_event_get);
		m_pcustom_event_get = NULL;
	}
}

// ��ȡ�¼�
custom_event_t *CEventList::get_event(void)
{
	if (m_pcustom_event_get != NULL)
	{
		destroy_event(m_pcustom_event_get);
		ua_free(m_pcustom_event_get);
		m_pcustom_event_get = NULL;
	}
	ua_mutex_lock(&m_mutex_list);
	if (m_list_event.GetCount() > 0)
	{
		if (!m_list_event.RemoveHead(m_pcustom_event_get))
			m_pcustom_event_get = NULL;
	}
	ua_mutex_unlock(&m_mutex_list);

	return m_pcustom_event_get;
}

// �����¼�����
void CEventList::destroy_event_list(void)
{
	custom_event_t		*p_event = NULL;

	ua_mutex_lock(&m_mutex_list);
	while (m_list_event.GetCount() > 0)
	{
		if (m_list_event.RemoveHead(p_event))
		{
			destroy_event(p_event);
			ua_free(p_event);
			p_event = NULL;
		}
	}
	ua_mutex_unlock(&m_mutex_list);
}

// �����¼�
void CEventList::destroy_event(custom_event_t *p_event)
{
	event_data_t		*p_cur_data = NULL, *p_next_data = NULL;

	if (p_event != NULL)
	{
		if (p_event->subclass != NULL)
		{
			ua_free(p_event->subclass);
			p_event->subclass = NULL;
		}
		for (p_next_data = p_event->data; p_next_data; )
		{
			p_cur_data = p_next_data;
			p_next_data = p_next_data->next;
			ua_free(p_cur_data->name);
			ua_free(p_cur_data->value);
			ua_free(p_cur_data);
		}
		p_event->data = NULL;
	}
}

// �����¼�(����ֵ��Ҫ�ⲿ�ͷ�)
custom_event_t *CEventList::create_event(const char *p_subclass)
{
	custom_event_t		*p_event = NULL;

	p_event = (custom_event_t *)ua_malloc(sizeof (custom_event_t));
	if (p_event != NULL)
	{
		memset(p_event, 0, sizeof (custom_event_t));
		p_event->subclass = string_dup(p_subclass);
		p_event->data = NULL;
	}

	return p_event;
}

// ����¼�����(name����Ϊ��, value����Ϊ��)
BOOL CEventList::event_add_data(custom_event_t *p_event, const char *p_name, const char *p_value)
{
	BOOL				b_add = FALSE;
	event_data_t		**pp_data = NULL;

	if (p_event != NULL && p_name != NULL)
	{
		for (pp_data = &p_event->data; *pp_data != NULL; pp_data = &(*pp_data)->next)
			;
		*pp_data = (event_data_t *)ua_malloc(sizeof (event_data_t));
		memset(*pp_data, 0, sizeof (event_data_t));
		(*pp_data)->name = string_dup(p_name);
		(*pp_data)->value = string_dup(p_value);
		(*pp_data)->next = NULL;
		b_add = TRUE;
	}

	return b_add;
}

// ���һ���¼�(�ṹ��)������(ע: �Ḵ���¼�, p_event����Ƕ�̬����, �����ⲿ�ͷ�)
BOOL CEventList::add_event(custom_event_t *p_event)
{
	BOOL				b_add = FALSE;
	custom_event_t		*p_new_event = NULL;
	event_data_t		*p_data = NULL;

	if (p_event != NULL && p_event->subclass != NULL)
	{
		p_new_event = create_event(p_event->subclass);
		if (p_new_event != NULL)
		{
			p_data = p_event->data;
			while (p_data != NULL)
			{
				event_add_data(p_new_event, p_data->name, p_data->value);
				p_data = p_data->next;
			}

			ua_mutex_lock(&m_mutex_list);
			m_list_event.AddTail(p_new_event);
			if (m_list_event.GetCount() > m_event_list_max)
			{
				if (m_list_event.RemoveHead(p_new_event))
				{
					destroy_event(p_new_event);
					ua_free(p_new_event);
					p_new_event = NULL;
				}
			}
			ua_mutex_unlock(&m_mutex_list);
			p_new_event = NULL;
			b_add = TRUE;

			if (m_psem_event != NULL)
				ua_sem_post(m_psem_event);
		}
	}

	return b_add;
}

// ���һ���¼�(����)������
BOOL CEventList::add_event(const char *p_subclass, const char *p_name, const char *p_value)
{
	BOOL				b_add = FALSE;
	custom_event_t		*p_event = NULL;

	if (p_subclass != NULL)
	{
		p_event = create_event(p_subclass);
		if (p_event != NULL)
		{
			event_add_data(p_event, p_name, p_value);
			ua_mutex_lock(&m_mutex_list);
			m_list_event.AddTail(p_event);
			if (m_list_event.GetCount() > m_event_list_max)
			{
				if (m_list_event.RemoveHead(p_event))
				{
					destroy_event(p_event);
					ua_free(p_event);
					p_event = NULL;
				}
			}
			ua_mutex_unlock(&m_mutex_list);
			p_event = NULL;
			b_add = TRUE;

			if (m_psem_event != NULL)
				ua_sem_post(m_psem_event);
		}
	}

	return b_add;
}
