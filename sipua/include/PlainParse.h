/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : PlainParse.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/08/23
* Description  : SDK的ESL Plain文本解释类
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
说明:
  数据类型1 : 
	label1|label2|lable3	// label
	value1|value2|value3	// item
  数据类型2 : 
	name1=value1			// item
	name2=value2			// item
	name3=value3			// item
  数据类型2 : 
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

	void set_delimit(char delimit);												// 设置变量分界符
	void init(char value_delimit, char item_delimit, BOOL has_label);			// 初始化
	void set_body(const char *p_body);											// 设置文本内容
	plain_item_t *get_next_item(void);											// 获取下一成员数据
	char *get_item_value(const char *p_name);									// 获取变量值(适用"数据类型2")
	int find_label_index(const char *p_label);									// 查找一个表头的序号
	void destroy_item(plain_item_t *p_item);									// 销毁list数据

private:
	BOOL get_item_text(const char *p_data, plain_item_t *p_item, int count);	// 获取一项数据

public:
	plain_item_t		m_plain_label;			// 标签

private:
	plain_item_t		m_plain_item;			// 成员
	const char			*m_pbody;				// plain内容
	int					m_body_len;				// 文本长度
	const char			*m_pfirst_item;			// 第一个成员位置
	const char			*m_pnext_item;			// 下一成员位置
	char				m_value_delimit;		// 变量分界符
	char				m_item_delimit;			// 项目记录分界符
	BOOL				m_has_label;			// 是否有标签
};

#endif // __PLAIN_PARSE_H__
 
