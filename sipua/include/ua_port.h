/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : ua_port.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/07/28
* Description  : UA 跨平台接口
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __UA_PORT_H__
#define __UA_PORT_H__

//-------------------- WIN32 --------------------
#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <limits.h>
//#define WIN32_LEAN_AND_MEAN	// 不让windows.h包含Winsock.h
#include <windows.h>
#include <WinDef.h>
#include <MMSystem.h>

#	define XC9000UA_EXPORTS
#	ifdef XC9000UA_EXPORTS
#		define UA_PUBLIC	__declspec(dllexport)
#	else 
#		define UA_PUBLIC	__declspec(dllimport)
#	endif

#define PATH_MAX		MAX_PATH

//----- STRING support begin
#ifndef strncasecmp
#	define strncasecmp					_strnicmp
#endif
#ifndef strcasecmp
#	define strcasecmp					_stricmp
#endif
#define DIRECTORY_SPLIT_CHAR			'\\'
//----- STRING support end

//----- net begin
#define ua_closesokcet					closesocket
//#define ua_usleep(us)					Sleep(us / 1000)
#ifdef __cplusplus
extern "C"
{
#endif
UA_PUBLIC void ua_usleep(int us);
#ifdef __cplusplus
}
#endif
//----- net end 

//----- I/O support begin
//----- I/O support end 

//-------------------- Linux --------------------
#else
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <limits.h>

#define UA_PUBLIC

//----- STRING support begin
#define DIRECTORY_SPLIT_CHAR			'/'
//----- STRING support end

//----- net begin
#define ua_closesokcet					::close
#define ua_usleep(us)					usleep(us)
//----- net end 

//----- I/O support begin
//----- I/O support end 

#endif // WIN32

//-------------------- WIN32 Linux 共用 --------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <fstream>
//#include <iostream>
//using namespace std; 
//#include <string>
//#include <exception>
#include <errno.h>
#include <fcntl.h>

#include "types.h"

#define ua_malloc			malloc
#define ua_free				free

//-------------------- thread --------------------
#ifdef WIN32
typedef struct ua_thread {
    HANDLE		h;
	unsigned	id;
} ua_thread_t;
#else
#	define ua_thread_t		pthread_t
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define				ua_thread_init		ua_thread_destroy
UA_PUBLIC BOOL		ua_thread_create(ua_thread_t *pthread, void *(*func)(void *), void *arg);	// 创建线程
UA_PUBLIC int		ua_thread_join(ua_thread_t *pthread);						// 等待线程退出
UA_PUBLIC void		ua_thread_exit(void);										// 退出线程
UA_PUBLIC void		ua_thread_destroy(ua_thread_t *pthread);					// 销毁线程
#ifdef __cplusplus
}
#endif

//-------------------- mutex --------------------
typedef struct ua_mutex {
#ifdef WIN32
#define UA_CRITICALSECTION_SPIN  4000
    CRITICAL_SECTION	h;
#else
    pthread_mutex_t     h;
#endif
} ua_mutex_t;

#ifdef __cplusplus
extern "C"
{
#endif

UA_PUBLIC BOOL			ua_mutex_init(ua_mutex_t *mut);						// 初始化互斥量
UA_PUBLIC BOOL			ua_mutex_destroy(ua_mutex_t *mut);					// 销毁互斥量
UA_PUBLIC void			ua_mutex_lock(ua_mutex_t *mut);						// 互斥量加锁
UA_PUBLIC void			ua_mutex_unlock(ua_mutex_t *mut);					// 互斥量解锁

#ifdef __cplusplus
}
#endif

//-------------------- timer --------------------
typedef struct timer_data {
    void			*p_param;
#ifdef WIN32
    HANDLE			timerid;
	UINT			timer_resolution;
#else
    timer_t			timerid;
#endif
    int				sival;
	struct ua_mutex	mut;
} timer_data_t;

#ifdef __cplusplus
extern "C"
{
#endif

UA_PUBLIC void      ua_get_time(struct timeval *ptv);						// 获取当前时间(距离1970年经历的秒数)
UA_PUBLIC long      ua_get_timeout(struct timeval *ptv_start);				// 当前时间与start时间的差(毫秒)
#ifdef WIN32
UA_PUBLIC BOOL		ua_create_timer_event(timer_data_t *p_timer_data, void (CALLBACK *p_callback)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR), \
							int due_time, int interval_msec, int sival);	// 创建一个定时器
UA_PUBLIC BOOL		ua_create_timer_queue(timer_data_t *p_timer_data, void (CALLBACK *p_callback)(void *, BOOL), \
							int due_time, int interval_msec, int sival);	// 创建一个定时器
UA_PUBLIC BOOL		ua_delete_timer_event(timer_data_t *p_timer_data, BOOL b_wait);// 删除一个定时器(注意:不能在定时器回调函数里调用)
UA_PUBLIC BOOL		ua_delete_timer_queue(timer_data_t *p_timer_data, BOOL b_wait);// 删除一个定时器(注意:不能在定时器回调函数里调用)
#else
#define ua_create_timer_event ua_create_timer
#define ua_create_timer_queue ua_create_timer
UA_PUBLIC BOOL		ua_create_timer(timer_data_t *p_timer_data, void(*p_callback)(union sigval), \
							int due_time, int interval_msec, int sival);	// 创建一个定时器
#define ua_delete_timer_event ua_delete_timer
#define ua_delete_timer_queue ua_delete_timer
UA_PUBLIC BOOL		ua_delete_timer(timer_data_t *p_timer_data, BOOL b_wait);// 删除一个定时器(注意:不能在定时器回调函数里调用)
#endif

#ifdef __cplusplus
}
#endif

//-------------------- semaphore --------------------
#ifdef WIN32
typedef struct ua_sem {
    HANDLE		h;
} ua_sem_t;
#else
#	define ua_sem_t		sem_t
#endif

#ifdef __cplusplus
extern "C"
{
#endif

UA_PUBLIC BOOL		ua_sem_init(ua_sem_t *psem, unsigned int value);			// 初始化信号
UA_PUBLIC BOOL		ua_sem_destroy(ua_sem_t *psem);								// 销毁信号
UA_PUBLIC void		ua_sem_post(ua_sem_t *psem);								// 信号量加1
UA_PUBLIC BOOL		ua_sem_wait(ua_sem_t *psem);								// 等待信号
UA_PUBLIC BOOL		ua_sem_trywait(ua_sem_t *psem);								// 查询信号

#ifdef __cplusplus
}
#endif

//-------------------- condition variable --------------------
#ifdef WIN32
typedef struct ua_cond {
  struct ua_mutex	mut;
  struct ua_sem		sem;
} ua_cond_t;
#else
typedef struct ua_cond {
  pthread_cond_t cv;
} ua_cond_t;
#endif

#ifdef __cplusplus
extern "C"
{
#endif
UA_PUBLIC BOOL		ua_cond_init(ua_cond_t *pcond);								// 初始化条件变量
UA_PUBLIC BOOL		ua_cond_destroy(ua_cond_t *pcond);							// 销毁条件变量
UA_PUBLIC void		ua_cond_signal(ua_cond_t *pcond);							// 条件变量加1
UA_PUBLIC BOOL		ua_cond_wait(ua_cond_t *pcond, ua_mutex_t *pmut);			// 等待条件
UA_PUBLIC BOOL		ua_cond_timedwait(ua_cond_t *pcond, ua_mutex_t *pmut, long mseconds);// 超时等待条件
#ifdef __cplusplus
}
#endif

//-------------------- socket --------------------
#ifdef __cplusplus
extern "C"
{
#endif
UA_PUBLIC BOOL		ua_is_multicast(const char *p_ip);							// 是否是组播地址
#ifdef __cplusplus
}
#endif


//-------------------- error --------------------
#define UA_SUCCESS               0
#define UA_UNDEFINED_ERROR      -1
#define UA_BADPARAMETER         -2
#define UA_WRONG_STATE          -3
#define UA_NOMEM                -4
#define UA_SYNTAXERROR          -5
#define UA_NOTFOUND             -6
#define UA_API_NOT_INITIALIZED  -7
#define UA_NO_NETWORK           -10
#define UA_PORT_BUSY            -11
#define UA_UNKNOWN_HOST         -12
#define UA_DISK_FULL            -30
#define UA_NO_RIGHTS            -31
#define UA_FILE_NOT_EXIST       -32
#define UA_TIMEOUT              -50
#define UA_TOOMUCHCALL          -51
#define UA_WRONG_FORMAT         -52
#define UA_NOCOMMONCODEC        -53

#endif // __UA_PORT_H__
