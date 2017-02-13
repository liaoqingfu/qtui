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
#include "ua_port.h"
#include <fstream>
#include <iostream>
using namespace std; 
#include <string>
#include <exception>

#include "log.h"

#ifdef WIN32

//----- timer
#include <time.h>
#include <sys/timeb.h>
#pragma comment(lib,"Winmm.lib")
#include <Mmsystem.h>

//----- thread
#include <process.h>

#else

#include <signal.h>
#include <arpa/inet.h>

#endif

//----------------- 1. 宏及结构本声明 -----------------

//----------------- 2. 变量定义 -----------------------

//----------------- 3. 函数声明 -----------------------

//----------------- 4. 函数定义 -----------------------
#ifdef WIN32
inline void ua_usleep(int us)
{
	static int			s_socket_sleep = socket(AF_INET, SOCK_DGRAM, 0);
    fd_set				fd_socket;
	struct timeval		time_sleep;

	if (s_socket_sleep != INVALID_SOCKET)
	{
		if (us <= 0)
			us = 1;
		time_sleep.tv_sec = us / 1000000;
		time_sleep.tv_usec = us % 1000000;
		FD_ZERO(&fd_socket);
		FD_SET(s_socket_sleep, &fd_socket);
		select(s_socket_sleep + 1, &fd_socket, NULL, NULL, &time_sleep);
	}
	else
	{
		if (us >= 1000)
			Sleep(us / 1000);
		else
			Sleep(1);
	}
}
#endif

//-------------------- mutex --------------------
//----- WIN32 
#ifdef WIN32
// 初始化互斥量
BOOL ua_mutex_init(ua_mutex_t *pmut)
{
	BOOL		b_init = FALSE;

	if (pmut != NULL)
		b_init = InitializeCriticalSectionAndSpinCount(&pmut->h, UA_CRITICALSECTION_SPIN);
	return b_init;
}

// 销毁互斥量
BOOL ua_mutex_destroy(ua_mutex_t *pmut)
{
	if (pmut != NULL)
		DeleteCriticalSection(&pmut->h);
	return TRUE;
}

// 互斥量加锁
void ua_mutex_lock(ua_mutex_t *pmut)
{
	if (pmut != NULL)
		EnterCriticalSection(&pmut->h);
}

// 互斥量解锁
void ua_mutex_unlock(ua_mutex_t *pmut)
{
	if (pmut != NULL)
		LeaveCriticalSection(&pmut->h);
}

//----- Linux 
#else
// 初始化互斥量
BOOL ua_mutex_init(ua_mutex_t *pmut)
{
	BOOL		b_init = FALSE;

	if (pmut != NULL)
	{
		if (pthread_mutex_init(&pmut->h, NULL) == 0)
			b_init = TRUE;
	}
	return b_init;
}

// 销毁互斥量
BOOL ua_mutex_destroy(ua_mutex_t *pmut)
{
	BOOL	b_destroy = FALSE;

	if (pmut != NULL)
	{
		if (pthread_mutex_destroy(&pmut->h) == 0)
			b_destroy = TRUE;
	}

	return b_destroy;
}

// 互斥量加锁
void ua_mutex_lock(ua_mutex_t *pmut)
{
	if (pmut != NULL)
		pthread_mutex_lock(&pmut->h);
}

// 互斥量解锁
void ua_mutex_unlock(ua_mutex_t *pmut)
{
	if (pmut != NULL)
		pthread_mutex_unlock(&pmut->h);
}
#endif // WIN32

//-------------------- timer --------------------
// 获取当前时间(距离1970年经历的秒数)
void ua_get_time(struct timeval *ptv)
{
#ifdef WIN32
	struct _timeb	timebuffer;
	if (ptv != NULL)
	{
		_ftime_s(&timebuffer);
		ptv->tv_sec = (long)timebuffer.time;
		ptv->tv_usec = timebuffer.millitm * 1000;
	}
#else
	struct timespec		ts_now;
	if (ptv != NULL)
	{
		clock_gettime(CLOCK_MONOTONIC, &ts_now);
		ptv->tv_sec = ts_now.tv_sec;
		ptv->tv_usec = ts_now.tv_nsec / 1000;
	}
#endif
}

// 当前时间与start时间的差(毫秒)
long ua_get_timeout(struct timeval *ptv_start)
{
    struct timeval  ts_end;
    unsigned long   timeout = 0;

	ua_get_time(&ts_end);
    timeout = (long)difftime(ts_end.tv_sec, ptv_start->tv_sec);
    if (ts_end.tv_usec >= ptv_start->tv_usec)
        timeout = timeout * 1000 + (ts_end.tv_usec - ptv_start->tv_usec) / 1000;
    else
        timeout = (timeout - 1) * 1000 + (1000000 + ts_end.tv_usec - ptv_start->tv_usec) / 1000;
    
    return timeout;
}

#ifdef WIN32
// 删除一个定时器(精确定时器)
BOOL ua_delete_timer_event(timer_data_t *p_timer_data, BOOL b_wait)
{
	BOOL		b_delete = FALSE;

	if (p_timer_data != NULL)
	{
		//ua_mutex_lock(&p_timer_data->mut);
		if (timeKillEvent((MMRESULT)p_timer_data->timerid) != TIMERR_NOERROR)
			printf_log(LOG_IS_ERR, "[ua_delete_timer(timeKillEvent)](%d) : %s\n", errno, strerror(errno));
		else
			p_timer_data->timerid = NULL;
		if (p_timer_data->timer_resolution != 0)
		{
			timeEndPeriod(p_timer_data->timer_resolution);
			p_timer_data->timer_resolution = 0;
		}
		//ua_mutex_unlock(&p_timer_data->mut);
		if (p_timer_data->timerid == NULL)
		{
			ua_mutex_destroy(&p_timer_data->mut);
			b_delete = TRUE;
		}
	}

	return b_delete;
}

// 删除一个定时器(非精确定时器)
BOOL ua_delete_timer_queue(timer_data_t *p_timer_data, BOOL b_wait)
{
	BOOL		b_delete = FALSE;
	if (p_timer_data != NULL)
	{
		//ua_mutex_lock(&p_timer_data->mut);
		if (b_wait)
		{
			if (DeleteTimerQueueTimer(NULL, p_timer_data->timerid, INVALID_HANDLE_VALUE))
				p_timer_data->timerid = NULL;	// 注意: 使用INVALID_HANDLE_VALUE参数,会一直等待,如果在定时器回调函数里删除定时器,后面的代码不会执行
		}
		else
		{
			DeleteTimerQueueTimer(NULL, p_timer_data->timerid, NULL);
			p_timer_data->timerid = NULL;
		}
		//ua_mutex_unlock(&p_timer_data->mut);
		if (p_timer_data->timerid == NULL)
		{
			ua_mutex_destroy(&p_timer_data->mut);
			b_delete = TRUE;
		}
	}

	return b_delete;
}

#else
// 删除一个定时器
BOOL ua_delete_timer(timer_data_t *p_timer_data, BOOL b_wait)
{
	BOOL		b_delete = FALSE;

	if (p_timer_data != NULL)
	{
		//ua_mutex_lock(&p_timer_data->mut);
        if (timer_delete(p_timer_data->timerid) == 0)
			p_timer_data->timerid = NULL;

		//ua_mutex_unlock(&p_timer_data->mut);
		if (p_timer_data->timerid == NULL)
		{
			ua_mutex_destroy(&p_timer_data->mut);
			b_delete = TRUE;
		}
	}

	return b_delete;
}
#endif

//----- WIN32
#ifdef WIN32
// 创建一个定时器(精确定时器)
BOOL ua_create_timer_event(timer_data_t *p_timer_data, void (CALLBACK *p_callback)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR), int due_time, int interval_msec, int sival)
{
    BOOL                b_result = FALSE;
	char	            p_strerr[PATH_MAX] = {0};
	TIMECAPS			tc;

    try
    {
		ua_mutex_init(&p_timer_data->mut);
        p_timer_data->sival = sival;
        if (p_timer_data->timerid != NULL)
            ua_delete_timer_event(p_timer_data, TRUE);
		p_timer_data->timer_resolution = 0;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
		{
			p_timer_data->timer_resolution = min(max(tc.wPeriodMin, (UINT)due_time), tc.wPeriodMax);
			timeBeginPeriod(p_timer_data->timer_resolution);
		}
		if (interval_msec == 0)
			p_timer_data->timerid = (HANDLE)timeSetEvent(due_time, p_timer_data->timer_resolution, (LPTIMECALLBACK)p_callback, (DWORD_PTR)p_timer_data, TIME_ONESHOT);
		else
			p_timer_data->timerid = (HANDLE)timeSetEvent(interval_msec, p_timer_data->timer_resolution, (LPTIMECALLBACK)p_callback, (DWORD_PTR)p_timer_data, TIME_PERIODIC);
		if (p_timer_data->timerid == NULL)
		{
            sprintf(p_strerr, "[ua_create_timer(timeSetEvent)](%d) : %s\n", errno, strerror(errno));
            throw 10;
		}
        
        b_result = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : error catch!\n");
    }

    return b_result;
}

// 创建一个定时器(非精确定时器)
BOOL ua_create_timer_queue(timer_data_t *p_timer_data, void (CALLBACK *p_callback)(void *, BOOL), int due_time, int interval_msec, int sival)
{
    BOOL                b_result = FALSE;
	char	            p_strerr[PATH_MAX] = {0};

    try
    {
		ua_mutex_init(&p_timer_data->mut);
        p_timer_data->sival = sival;
        if (p_timer_data->timerid != NULL)
            ua_delete_timer_queue(p_timer_data, TRUE);
		if (CreateTimerQueueTimer(&p_timer_data->timerid, NULL, (WAITORTIMERCALLBACK)p_callback, (PVOID)p_timer_data, due_time, \
			interval_msec, WT_EXECUTEDEFAULT) == 0)
		{
            sprintf(p_strerr, "[ua_create_timer(CreateTimerQueueTimer)](%d) : %s\n", errno, strerror(errno));
            throw 10;
        }
        
        b_result = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : error catch!\n");
    }

    return b_result;
}

//----- Linux
#else
// 创建一个定时器
BOOL ua_create_timer(timer_data_t *p_timer_data, void(*p_callback)(union sigval), int due_time, int interval_msec, int sival)
{
    BOOL                b_result = FALSE;
	char	            p_strerr[PATH_MAX] = {0};
	struct sigevent     sig_event;
    struct itimerspec   itimer_spec;

    try
    {
		ua_mutex_init(&p_timer_data->mut);
        memset(&sig_event, 0, sizeof(struct sigevent));
        sig_event.sigev_value.sival_ptr = p_timer_data;
        sig_event.sigev_notify = SIGEV_THREAD;
        sig_event.sigev_notify_function = p_callback;
        p_timer_data->sival = sival;
        if (p_timer_data->timerid != NULL)
            ua_delete_timer_event(p_timer_data, TRUE);
        if (timer_create(CLOCK_REALTIME, &sig_event, &p_timer_data->timerid) == -1)
        {
            sprintf(p_strerr, "[ua_create_timer(timer_create)](%d) : %s\n", errno, strerror(errno));
            throw 10;
        }
        itimer_spec.it_interval.tv_sec = interval_msec / 1000;
        itimer_spec.it_interval.tv_nsec = (interval_msec % 1000) * 1000000;
        itimer_spec.it_value.tv_sec = due_time / 1000;
        itimer_spec.it_value.tv_nsec = (due_time % 1000) * 1000000;
        if (timer_settime(p_timer_data->timerid, 0, &itimer_spec, NULL) == -1)
        {
            sprintf(p_strerr, "[ua_create_timer(timer_settime)](%d) : %s\n", errno, strerror(errno));
            throw 11;
        }
        
        b_result = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 11:
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[ua_create_timer()] : error catch!\n");
    }

    return b_result;
}
#endif // WIN32

//-------------------- thread --------------------
//----- WIN32
#ifdef WIN32
// 创建线程
BOOL ua_thread_create(ua_thread_t *pthread, void *(*func)(void *), void *arg)
{
	BOOL	b_create = FALSE;

	if (pthread != NULL)
	{
		pthread->h = (HANDLE)_beginthreadex(NULL,    // default security attr
											0,       // use default one
											(unsigned (__stdcall *)(void *))func, arg, 0, &(pthread->id));
		if (pthread->h != 0)
			b_create = TRUE;
	}
	return b_create;
}

// 等待线程退出
int ua_thread_join(ua_thread_t *pthread)
{
	int			result = UA_SUCCESS, status;

	if (pthread == NULL)
		result = UA_BADPARAMETER;
	else
	{
		status = WaitForSingleObject(pthread->h, INFINITE);
		if (status != WAIT_OBJECT_0)
			result = UA_UNDEFINED_ERROR;
		CloseHandle(pthread->h);
	}

	return result;
}

// 退出线程
void ua_thread_exit(void)
{
	_endthreadex(0);
}

// 销毁线程
void ua_thread_destroy(ua_thread_t *pthread)
{
	if (pthread != NULL)
	{
		pthread->h = INVALID_HANDLE_VALUE;
		pthread->id = 0;
	}
}

//----- Linux
#else
// 创建线程
BOOL ua_thread_create(ua_thread_t *pthread, void *(*func)(void *), void *arg)
{
	BOOL			b_create = FALSE;
    pthread_attr_t	attr;

	if (pthread != NULL)
	{
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); // 与系统中所有线程一起竞争CPU时间
		if (pthread_create(pthread, &attr, func, arg) == 0)
			b_create = TRUE;
	}
	return b_create;
}

// 等待线程退出
int ua_thread_join(ua_thread_t *pthread)
{
	int			result = UA_SUCCESS;

	if (pthread == NULL)
		result = UA_BADPARAMETER;
	else
		result = pthread_join(*pthread, NULL);

	return result;
}

// 退出线程
void ua_thread_exit(void)
{
	pthread_exit(NULL);
}

// 销毁线程
void ua_thread_destroy(ua_thread_t *pthread)
{
	if (pthread != NULL)
		*pthread = 0;
}

#endif // WIN32

//-------------------- semaphore --------------------
//----- WIN32
#ifdef WIN32
// 初始化信号
BOOL ua_sem_init(ua_sem_t *psem, unsigned int value)
{
	BOOL		b_init = FALSE;

	if (psem != NULL)
	{
		psem->h = CreateSemaphore(NULL, value, LONG_MAX, NULL);
		if (psem->h != NULL)
			b_init = TRUE;
	}

	return b_init;
}

// 销毁信号
BOOL ua_sem_destroy(ua_sem_t *psem)
{
	BOOL	b_destroy = FALSE;

	if (psem != NULL)
	{
		if (CloseHandle(psem->h))
		{
			psem->h = INVALID_HANDLE_VALUE;
			b_destroy = TRUE;
		}
	}

	return b_destroy;
}

// 信号量加1
void ua_sem_post(ua_sem_t *psem)
{
	if (psem != NULL)
		ReleaseSemaphore(psem->h, 1, NULL);
}

// 等待信号
BOOL ua_sem_wait(ua_sem_t *psem)
{
	BOOL	b_sem = FALSE;

	if (psem != NULL)
	{
		if (WaitForSingleObject(psem->h, INFINITE) == WAIT_OBJECT_0)
			b_sem = TRUE;
	}

	return b_sem;
}

// 查询信号
BOOL ua_sem_trywait(ua_sem_t *psem)
{
	BOOL	b_sem = FALSE;

	if (psem != NULL)
	{
		if (WaitForSingleObject(psem->h, 0) == WAIT_OBJECT_0)
			b_sem = TRUE;
	}

	return b_sem;
}

#else
//----- Linux
// 初始化信号
BOOL ua_sem_init(ua_sem_t *psem, unsigned int value)
{
	BOOL		b_init = FALSE;

	if (psem != NULL)
	{
		if (sem_init(psem, 0, value) == 0)
			b_init = TRUE;
	}

	return b_init;
}

// 销毁信号
BOOL ua_sem_destroy(ua_sem_t *psem)
{
	BOOL	b_destroy = FALSE;

	if (psem != NULL)
	{
		if (sem_destroy(psem) == 0)
			b_destroy = TRUE;
	}

	return b_destroy;
}

// 信号量加1
void ua_sem_post(ua_sem_t *psem)
{
	if (psem != NULL)
		sem_post(psem);
}

// 等待信号
BOOL ua_sem_wait(ua_sem_t *psem)
{
	BOOL	b_sem = FALSE;

	if (psem != NULL)
	{
		if (sem_wait(psem) == 0)
			b_sem = TRUE;
	}

	return b_sem;
}

// 查询信号
BOOL ua_sem_trywait(ua_sem_t *psem)
{
	BOOL	b_sem = FALSE;

	if (psem != NULL)
	{
		if (sem_trywait(psem) == 0)
			b_sem = TRUE;
	}

	return b_sem;
}

#endif // WIN32

//-------------------- condition variable --------------------
//----- WIN32
#ifdef WIN32
// 初始化条件变量
BOOL ua_cond_init(ua_cond_t *pcond)
{
	BOOL		b_init = FALSE;

	if (pcond != NULL && ua_mutex_init(&pcond->mut))
	{
		if(ua_sem_init(&pcond->sem, 0))
			b_init = TRUE;
		else
			ua_mutex_destroy(&pcond->mut);
	}

	return b_init;
}

// 销毁条件变量
BOOL ua_cond_destroy(ua_cond_t *pcond)
{
	BOOL	b_destroy = FALSE;

	if (pcond != NULL)
	{
		b_destroy = ua_sem_destroy(&pcond->sem);
		b_destroy &= ua_mutex_destroy(&pcond->mut);
	}

	return b_destroy;
}

// 条件变量加1
void ua_cond_signal(ua_cond_t *pcond)
{
	if (pcond != NULL)
		ua_sem_post(&pcond->sem);
}

// 等待条件
BOOL ua_cond_wait(ua_cond_t *pcond, ua_mutex_t *pmut)
{
	BOOL	b_cond = FALSE;

	if (pcond != NULL && pmut != NULL)
	{
		ua_mutex_lock(&pcond->mut);
		ua_mutex_unlock(pmut);
		b_cond = ua_sem_wait(&pcond->sem);
		ua_mutex_lock(pmut);
		ua_mutex_unlock(&pcond->mut);
	}

	return b_cond;
}

// 超时等待条件
BOOL ua_cond_timedwait(ua_cond_t *pcond, ua_mutex_t *pmut, long mseconds)
{
	BOOL	b_cond = FALSE;

	if (pcond != NULL && pmut != NULL)
	{
		ua_mutex_lock(&pcond->mut);
		ua_mutex_unlock(pmut);
		if (WaitForSingleObject(&pcond->sem, mseconds) == WAIT_OBJECT_0)
			b_cond = TRUE;
		ua_mutex_lock(pmut);
		ua_mutex_unlock(&pcond->mut);
	}

	return b_cond;
}

#else
//----- Linux
// 初始化条件变量
BOOL ua_cond_init(ua_cond_t *pcond)
{
	BOOL		b_init = FALSE;

	if (pcond != NULL)
	{
		if(pthread_cond_init(&pcond->cv, NULL) == 0)
			b_init = TRUE;
	}

	return b_init;
}

// 销毁条件变量
BOOL ua_cond_destroy(ua_cond_t *pcond)
{
	BOOL	b_destroy = FALSE;

	if (pcond != NULL)
	{
		if (pthread_cond_destroy(&pcond->cv) == 0)
			b_destroy = TRUE;
	}

	return b_destroy;
}

// 条件变量加1
void ua_cond_signal(ua_cond_t *pcond)
{
	if (pcond != NULL)
		pthread_cond_signal(&pcond->cv);
}

// 等待条件
BOOL ua_cond_wait(ua_cond_t *pcond, ua_mutex_t *pmut)
{
	BOOL	b_cond = FALSE;

	if (pcond != NULL && pmut != NULL)
	{
		if (pthread_cond_wait(&pcond->cv, &pmut->h) == 0)
			b_cond = TRUE;
	}

	return b_cond;
}

// 超时等待条件
BOOL ua_cond_timedwait(ua_cond_t *pcond, ua_mutex_t *pmut, long mseconds)
{
	BOOL				b_cond = FALSE;
	long				nseconds;
	struct timespec		abstime;

	if (pcond != NULL && pmut != NULL)
	{
		clock_gettime(CLOCK_MONOTONIC, &abstime);
		nseconds = abstime.tv_nsec + (mseconds % 1000) * 1000000;
		abstime.tv_nsec = nseconds % 1000000000;
		abstime.tv_sec += (mseconds / 1000) + (nseconds / 1000000000);

		if (pthread_cond_timedwait(&pcond->cv, &pmut->h, &abstime) == 0)
			b_cond = TRUE;
	}

	return b_cond;
}

#endif // WIN32

// 是否是组播地址
BOOL ua_is_multicast(const char *p_ip)
{
	BOOL				b_multicast = FALSE;
	struct sockaddr_in	addr;
	unsigned char		addr_net;

	if (p_ip != NULL)
	{
		addr.sin_addr.s_addr = inet_addr(p_ip);
		addr_net = (addr.sin_addr.s_addr & 0xFF);
		if (addr_net >= 224 && addr_net <= 239)
			b_multicast = TRUE;
	}

	return b_multicast;
}
