/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : log.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : 日志类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "ua_port.h"
#include <fstream>
#include <iostream>
using namespace std; 
#include <string>
#include <exception>

#include <limits.h>
#include <sys/stat.h>
#include <time.h>

#include "log.h"
#include "ua_global.h"

#ifdef WIN32
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#endif

CLog     g_log;

void set_log_file(int log_level)
{
	g_log.m_log_file_level = log_level;
}

void set_console(int log_level)
{
	g_log.m_console_level = log_level;
}

const char g_log_level[LOG_IS_ERR + 2][10] = {
	"[DEBUG  ]",
	"[INFO   ]",
	"[WARNING]",
	"[RECORD ]",
	"[ERROR  ]",
	"[UNKNOWN]",
} ;
void printf_log(int log_level, const char *fmt, ...)
{
	char			*log = NULL, *level = NULL;
	int				len;
    va_list         args;
    
    va_start(args, fmt);
	len = vsnprintf(NULL, 0, fmt, args);
	log = (char *)ua_malloc(len + 1);
    vsnprintf(log, len + 1, fmt, args);
    va_end(args);
	
	if (log_level < 0 || log_level > LOG_IS_ERR)
		log_level = LOG_IS_ERR + 1;
	len = snprintf(NULL, 0, "%s%s", g_log_level[log_level], log);
	level = (char *)ua_malloc(len + 1);
    snprintf(level, len + 1, "%s%s", g_log_level[log_level], log);

	if (log_level >= g_log.m_log_file_level)
		g_log.write(level);
	if (log_level >= g_log.m_console_level)
		fprintf(stdout, "%s", level);
	if (g_log.m_log_callback != NULL)
		g_log.m_log_callback(log_level, level, g_log.m_callback_param);

	if (log != NULL)
		ua_free(log);
	if (level != NULL)
		ua_free(level);
}

// missing string printf
// this is safe and convenient but not exactly efficient
string format(const char* fmt, ...)
{
    int         size = 512, nsize;
    char        *buffer = 0;
    va_list     vl;
    
    buffer = (char *)ua_malloc(size);
    va_start(vl, fmt);
    nsize = vsnprintf(buffer, size, fmt, vl);
    if(size <= nsize)
    {   //fail delete buffer and try again
        ua_free(buffer);
        buffer = 0;
        buffer = (char *)ua_malloc(nsize + 1); // +1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    ua_free(buffer);
    
    return ret;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog(const char *file_name)
{
	string		    month;
	string          str_read, str_write;

	//变量初始化
	m_log_callback = NULL;
	m_callback_param = NULL;
	m_log_file_level = LOG_IS_WARNING;
	m_console_level = LOG_IS_DEBUG;
	m_month = 0;
	m_file_name = file_name;

	open_file("Program running");
}

BOOL CLog::open_file(const char *log_open)
{
	char			path[PATH_MAX];
	time_t          now;
	struct tm       *p_tm_local;
	string		    year, month;
	string          str_read, str_write;
	BOOL			b_open = FALSE;

	time(&now);
	p_tm_local = localtime(&now);
	// 打开日志文件
	get_exe_path(path, PATH_MAX);
	m_file_path = path;
	m_file_path = m_file_path.substr(0, m_file_path.find_last_of(DIRECTORY_SPLIT_CHAR) + 1);
	// LOG_FOLDER_NAME文件夹创建
	m_file_path += LOG_FOLDER_NAME;
#ifdef WIN32
	WIN32_FIND_DATA	hFindFileData;
	HANDLE			hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(m_file_path.c_str(), &hFindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		CreateDirectory(m_file_path.c_str(), NULL);
	else
		FindClose(hFind);
#else
	int			    find;
	find = access(m_file_path.c_str(), F_OK);
	if (find == -1)
	{
		if (mkdir(m_file_path.c_str(), 0764) == -1)
		{
		    printf("[CLog::CLog(mkdir)](%d) : %s\n", errno, strerror(errno));
		}
    }
#endif
	m_file_path += DIRECTORY_SPLIT_CHAR;
	// 年份文件夹创建
	year = format("%d", p_tm_local->tm_year + 1900);
	m_file_path += year;
#ifdef WIN32
	hFind = FindFirstFile(m_file_path.c_str(), &hFindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		CreateDirectory(m_file_path.c_str(), NULL);
	else
		FindClose(hFind);
#else
	find = access(m_file_path.c_str(), F_OK);
	if (find == -1)
	{
		if (mkdir(m_file_path.c_str(), 0764) == -1)
		{
		    printf("[CLog::CLog(mkdir)](%d) : %s\n", errno, strerror(errno));
		}
    }
#endif
	m_file_path += DIRECTORY_SPLIT_CHAR;

	m_month = p_tm_local->tm_mon;
	m_file_path += m_file_name;
	month = format("_%02d", p_tm_local->tm_mon + 1);
	m_file_path.insert(m_file_path.length() - 4, month.c_str());
	m_file.open(m_file_path.c_str(), ios::binary | ios::in | ios::out | ios::app);

	if (m_file.is_open())
	{
		//文件头标志检查
        m_file.seekg(ios::beg);
		getline(m_file, str_read);
		if (str_read.compare(LOG_FILE_FLAG) != 0)
		{
		    m_file.clear();
			m_file.seekp(ios::beg);
			m_file << LOG_FILE_FLAG << _CRLF;
			m_file << "Log file" << _CRLF;
			m_file << "Program name : ";
			m_file << path << _CRLF;
			m_file << "Create date : ";
			strftime(path, PATH_MAX, "%Y-%m-%d %X", p_tm_local);
			m_file << path << _CRLF;
			m_file << "****************/" << _CRLF;
		}

		//本次运行
		m_file.seekp(ios::end);
        m_file << _CRLF << LOG_FILE_NEW_RUN << _CRLF;
		write(log_open);

		b_open = TRUE;
	}

	return b_open;
}

void CLog::close_file(const char *log_close)
{
	if (m_file.is_open())
	{
		write(log_close);
		m_file << LOG_FILE_NEW_END << _CRLF;
		m_file.close();
	}
}

CLog::~CLog()
{
	close_file("Program normal end");
}

void CLog::get_cur_time(string &str_time)
{
	char			p_time[PATH_MAX];
	time_t          now;
	struct tm       *p_tm_local;
	char			p_log_close[64];
	
	time(&now);
	p_tm_local = localtime(&now);
	strftime(p_time, PATH_MAX, "%Y-%m-%d %X", p_tm_local);
	str_time = p_time;
	if (m_month != p_tm_local->tm_mon)
	{	// 不是同一个月
		sprintf(p_log_close, "month change : %d -> %d", m_month + 1, p_tm_local->tm_mon + 1);
		m_month = p_tm_local->tm_mon;
		close_file(p_log_close);
		open_file(p_log_close);
	}
}

void CLog::write(char *p_buf, BOOL enter)
{
    int     len;

	if (!m_file.is_open() || p_buf == NULL)
		return;
	len = strlen(p_buf);
	if (len > 0)
	{
		string      str_time;
		get_cur_time(str_time);
		str_time += " : ";
		str_time += p_buf;
		if (enter && p_buf[len - 1] != '\n')
			str_time += _CRLF;
		m_file.seekp(ios::end);
		m_file << str_time.c_str();
		m_file.flush();
	}
}

void CLog::write(const string str_log, BOOL enter)
{
    int     len;

	if (!m_file.is_open())
		return;
	len = str_log.length();
	if (len > 0)
	{
		string      str_time;
		get_cur_time(str_time);
		str_time += " : ";
		str_time += str_log;
		if (enter && str_log.at(len - 1) != '\n')
			str_time += _CRLF;
		m_file.seekp(ios::end);
		m_file << str_time.c_str();
		m_file.flush();
	}
}

// 设置log输出回调函数
void CLog::set_callback(void(*p_callback)(int, char *, void *), void *p_param)
{
	m_log_callback = p_callback;
	m_callback_param = p_param;
}



