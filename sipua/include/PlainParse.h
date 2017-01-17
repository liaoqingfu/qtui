/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : PlainParse.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/08/23
* Description  : SDK��ESL Plain�ı�������
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
˵��:
  ��������1 : 
	label1|label2|lable3	// label
	value1|value2|value3	// item
  ��������2 : 
	name1=value1			// item
	name2=value2			// item
	name3=value3			// item
  ��������2 : 
	name1=value1;name2=value2;name3=value3

*/

#ifndef __PLAIN_PARSE_H__
#define __PLAIN_PARSE_H__

typedef struct plain_item {
	char	**p_text;
	int		count;
} plain_item_t;

typedef struct plain_param {
	char	*name;
	char	*value;
} plain_param_t;

class CPlainParse
{
public:
	CPlainParse();
	~CPlainParse();

	void set_delimit(char delimit);												// ���ñ����ֽ��
	void init(char value_delimit, char item_delimit, BOOL has_label);			// ��ʼ��
	void set_body(const char *p_body);											// �����ı�����
	plain_item_t *get_next_item(void);											// ��ȡ��һ��Ա����
	char *get_item_value(const char *p_name);									// ��ȡ����ֵ(����"��������2")
	int find_label_index(const char *p_label);									// ����һ����ͷ�����
	void destroy_item(plain_item_t *p_item);									// ����list����

private:
	BOOL get_item_text(const char *p_data, plain_item_t *p_item, int count);	// ��ȡһ������

public:
	plain_item_t		m_plain_label;			// ��ǩ

private:
	plain_item_t		m_plain_item;			// ��Ա
	const char			*m_pbody;				// plain����
	int					m_body_len;				// �ı�����
	const char			*m_pfirst_item;			// ��һ����Աλ��
	const char			*m_pnext_item;			// ��һ��Աλ��
	char				m_value_delimit;		// �����ֽ��
	char				m_item_delimit;			// ��Ŀ��¼�ֽ��
	BOOL				m_has_label;			// �Ƿ��б�ǩ
};

#endif // __PLAIN_PARSE_H__
 
