/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : log.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : ��־�����
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
// Log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOG_H__
#define __LOG_H__

#define LOG_FOLDER_NAME		"log"				        // �ļ���
#define LOG_FILE_NAME		"log_main.txt"			    // �ļ���
#define LOG_FILE_FLAG		"/****************"		    // �ļ�ͷ��־
#define LOG_FILE_NEW_RUN	"/***** run *****/"		    // ����ʼ��־
#define LOG_FILE_NEW_END	"/***** end *****/"		    // ���������־
#define LOG_FILE_VAR_FALG	"# Variable :"			    // �ļ�������־

#define _CRLF				"\n"						// Carriage-Return Line-Feed �س�����

#include <fstream>
using namespace std; 
#include <string>

class CLog  
{
public:
	CLog(const char *file_name = LOG_FILE_NAME);
	virtual ~CLog();
	void write(const string str_log, BOOL enter = TRUE);
	void write(char *p_buf, BOOL enter = TRUE);
	void set_callback(void(*p_callback)(int, char *, void *), void *p_param);		// ����log����ص�����

	void		(*m_log_callback)(int, char *, void *);		// log����ص�����
	void		*m_callback_param;							// �ص���������
	int			m_log_file_level;							// ����Ϊ�ļ��ĵȼ�(Ĭ��LOG_IS_WARNING)
	int			m_console_level;							// ����̨��ӡ��Ϣ�ĵȼ�(Ĭ��LOG_IS_DEBUG)
private:
	void get_cur_time(string &str_time);
	BOOL open_file(const char *log_open);
	void close_file(const char *log_close);

private:
	fstream		m_file;										// ��־�ļ�
	string	    m_file_name;								// ��־�ļ���
	string	    m_file_path;								// ��־�ļ�·�����ļ���
	int			m_month;									// ��ǰ��־�·�
};

#define LOG_IS_DEBUG    0
#define LOG_IS_INFO     1
#define LOG_IS_WARNING  2
#define LOG_IS_RECORD   3
#define LOG_IS_ERR      4	// ��ֵ���������ֵ
extern CLog     g_log;

#ifdef __cplusplus
extern "C"
{
#endif

void printf_log(int log_level, const char *fmt, ...); 
void set_log_file(int log_level);
void set_console(int log_level);

#ifdef __cplusplus
}
#endif

UA_PUBLIC string    format(const char* fmt, ...);

#endif // __LOG_H__
