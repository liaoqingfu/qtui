/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : log.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : 日志类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
// Log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOG_H__
#define __LOG_H__

#define LOG_FOLDER_NAME		"log"				        // 文件夹
#define LOG_FILE_NAME		"log_main.txt"			    // 文件名
#define LOG_FILE_FLAG		"/****************"		    // 文件头标志
#define LOG_FILE_NEW_RUN	"/***** run *****/"		    // 程序开始标志
#define LOG_FILE_NEW_END	"/***** end *****/"		    // 程序结束标志
#define LOG_FILE_VAR_FALG	"# Variable :"			    // 文件变量标志

#define _CRLF				"\n"						// Carriage-Return Line-Feed 回车换行

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
	void set_callback(void(*p_callback)(int, char *, void *), void *p_param);		// 设置log输出回调函数

	void		(*m_log_callback)(int, char *, void *);		// log输出回调函数
	void		*m_callback_param;							// 回调函数参数
	int			m_log_file_level;							// 保存为文件的等级(默认LOG_IS_WARNING)
	int			m_console_level;							// 控制台打印信息的等级(默认LOG_IS_DEBUG)
private:
	void get_cur_time(string &str_time);
	BOOL open_file(const char *log_open);
	void close_file(const char *log_close);

private:
	fstream		m_file;										// 日志文件
	string	    m_file_name;								// 日志文件名
	string	    m_file_path;								// 日志文件路径及文件名
	int			m_month;									// 当前日志月份
};

#define LOG_IS_DEBUG    0
#define LOG_IS_INFO     1
#define LOG_IS_WARNING  2
#define LOG_IS_RECORD   3
#define LOG_IS_ERR      4	// 此值必须是最大值
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
