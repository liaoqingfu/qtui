/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : PlainParse.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/08/23
* Description  : SDK的ESL Plain文本解释类
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#include "ua_port.h"
#include "PlainParse.h"

CPlainParse::CPlainParse()
{
	m_pbody = NULL;
	m_body_len = 0;
	m_pnext_item = NULL;
	m_pfirst_item = NULL;
	m_value_delimit = '|';
	m_item_delimit = '\n';
	m_has_label = TRUE;
	m_plain_label.count = 0;
	m_plain_item.count = 0;
}

CPlainParse::~CPlainParse()
{
	destroy_item(&m_plain_label);
	destroy_item(&m_plain_item);
	m_pbody = NULL;
	m_body_len = 0;
	m_pnext_item = NULL;
	m_pfirst_item = NULL;
}

// 初始化
void CPlainParse::init(char value_delimit, char item_delimit, BOOL has_label)
{
	m_value_delimit = value_delimit;
	m_item_delimit = item_delimit;
	m_has_label = has_label;
}

// 销毁list数据
void CPlainParse::destroy_item(plain_item_t *p_item)
{
	int			i;
	if (p_item->count > 0)
	{
		for (i = 0; i < p_item->count; i++)
		{
			if (p_item->p_text[i] != NULL)
			{
				ua_free(p_item->p_text[i]);
				p_item->p_text[i] = NULL;
			}
		}
		ua_free(p_item->p_text);
	}
	p_item->p_text = NULL;
	p_item->count = 0;
}

// 设置分界符
void CPlainParse::set_delimit(char delimit)
{
	m_value_delimit = delimit;
}

// 设置文本内容
void CPlainParse::set_body(const char *p_body)
{
	const char	*p_item_delimit = NULL;
	char		*p_delimit = NULL;
	char		*p_label = NULL;
	int			i, len, count;

	if (p_body != NULL)
	{
		destroy_item(&m_plain_label);
		destroy_item(&m_plain_item);
		m_pbody = p_body;
		m_body_len = strlen(m_pbody);
		m_pnext_item = m_pfirst_item = p_body;
		// 创建表头
		p_item_delimit = strchr(m_pbody, m_item_delimit);
		if (p_item_delimit != NULL)
		{
			if (m_has_label)
				m_pnext_item = m_pfirst_item = p_item_delimit + sizeof (m_item_delimit);
			len = p_item_delimit - m_pbody;
			p_label = (char *)ua_malloc(len + 1);
			memcpy(p_label, m_pbody, len);
			p_label[len] = '\0';
			count = 1;
			for (i = 0; i < len; i++)
			{
				if (p_label[i] == m_value_delimit)
					count++;
			}
			get_item_text(p_label, &m_plain_label, count);
			ua_free(p_label);
			p_label = NULL;
		}
		else
		{
			if (m_has_label)
				m_pnext_item = m_pfirst_item = NULL;
			len = strlen(m_pbody);
			p_label = (char *)ua_malloc(len + 1);
			memcpy(p_label, m_pbody, len);
			p_label[len] = '\0';
			count = 1;
			for (i = 0; i < len; i++)
			{
				if (p_label[i] == m_value_delimit)
					count++;
			}
			get_item_text(p_label, &m_plain_label, count);
			ua_free(p_label);
			p_label = NULL;
		}
	}
}

// 获取一项数据
BOOL CPlainParse::get_item_text(const char *p_data, plain_item_t *p_item, int count)
{
	BOOL		result = FALSE;
	int			i, label, len;
	const char	*p_begin = NULL, *p_end = NULL;

	if (p_data != NULL && p_item != NULL && count > 0)
	{
		destroy_item(p_item);
		p_item->count = count;
		p_item->p_text = (char **)ua_malloc(sizeof (char *) * count);
		for (i = 0; i < count; i++)
			p_item->p_text[i] = NULL;

		len = strlen(p_data);
		p_begin = p_end = p_data;
		label = 0;
		do
		{
			if (*p_end == m_value_delimit)
			{
				while (*p_begin == ' ' && p_begin < p_end)		// 删除字段前的空格
					p_begin++;
				p_item->p_text[label] = (char *)ua_malloc(p_end - p_begin + 1);
				if (p_begin != p_end)
					memcpy(p_item->p_text[label], p_begin, p_end - p_begin);
				p_item->p_text[label][p_end - p_begin] = '\0';
				p_begin = p_end + 1;
				label++;
			}
			p_end++;
		} while (p_end < p_data + len && label < count);
		while (label < count)
		{
			while (*p_begin == ' ' && p_begin < p_end)		// 删除字段前的空格
				p_begin++;
			p_item->p_text[label] = (char *)ua_malloc(p_end - p_begin + 1);
			if (p_begin != p_end)
				memcpy(p_item->p_text[label], p_begin, p_end - p_begin);
			p_item->p_text[label][p_end - p_begin] = '\0';
			p_begin = p_end;
			label++;
		}
		result = TRUE;
	}

	return result;
}

// 查找一个表头的序号
int CPlainParse::find_label_index(const char *p_label)
{
	int		index = -1;

	if (m_pbody != NULL && m_plain_label.count > 0)
	{
		for (index = 0; index < m_plain_label.count; index++)
		{
			if (strcasecmp(p_label, m_plain_label.p_text[index]) == 0)
				break;
		}
		if (index >= m_plain_label.count)
			index = -1;
	}

	return index;
}

// 获取变量值
char *CPlainParse::get_item_value(const char *p_name)
{
	char				*p_value = NULL, *p_label = NULL;
	int					len_name, len_label;
	const char			*p_item_delimit = NULL, *p_cur_item = NULL;

	if (m_pbody != NULL && m_pfirst_item != NULL && p_name != NULL)
	{
		destroy_item(&m_plain_item);

		len_name = strlen(p_name);
		p_cur_item = m_pbody;
		do
		{
			if (strncasecmp(p_cur_item, p_name, len_name) == 0)
			{
				p_item_delimit = strchr(p_cur_item, m_item_delimit);
				if (p_item_delimit == NULL)
					len_label = strlen(p_cur_item);
				else
					len_label = p_item_delimit - p_cur_item;
				p_label = (char *)ua_malloc(len_label + 1);
				memcpy(p_label, p_cur_item, len_label);
				p_label[len_label] = '\0';

				get_item_text(p_label, &m_plain_item, 2);
				p_value = m_plain_item.p_text[1];
				ua_free(p_label);
				p_label = NULL;

				p_item_delimit = NULL;
			}
			else
			{
				p_item_delimit = strchr(p_cur_item, m_item_delimit);
				if (p_item_delimit != NULL)
					p_cur_item = p_item_delimit + sizeof (m_item_delimit);
			}
		} while (p_item_delimit != NULL);
	}

	return p_value;
}

// 获取下一成员数据
plain_item_t *CPlainParse::get_next_item(void)
{
	plain_item_t		*p_item = NULL;
	const char			*p_item_delimit = NULL, *p_cur_item = NULL;
	char				*p_label = NULL;
	int					len;

	if (m_pbody != NULL && m_pnext_item != NULL && m_pnext_item < m_pbody + m_body_len)
	{
		destroy_item(&m_plain_item);

		p_cur_item = m_pnext_item;
		p_item_delimit = strchr(m_pnext_item, m_item_delimit);
		if (p_item_delimit == NULL)
		{
			p_item_delimit = m_pnext_item + strlen(m_pnext_item);
			m_pnext_item = p_item_delimit;
		}
		else
			m_pnext_item = p_item_delimit + sizeof (m_item_delimit);
		len = p_item_delimit - p_cur_item;
		p_label = (char *)ua_malloc(len + 1);
		memcpy(p_label, p_cur_item, len);
		p_label[len] = '\0';

		get_item_text(p_label, &m_plain_item, m_plain_label.count);
		ua_free(p_label);
		p_label = NULL;

		p_item = &m_plain_item;
	}

	return p_item;
}
