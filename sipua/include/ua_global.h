/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : ua_global.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : 全局共用函数
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef _UA_GLOBAL_H
#define _UA_GLOBAL_H

#include <signal.h>

#include "types.h"

//----------------- 1. 宏及结构本声明 -----------------

//----------------- 2. 变量声明 -----------------------

//----------------- 3. 函数声明 -----------------------
#ifdef __cplusplus
extern "C"
{
#endif
UA_PUBLIC char      *get_exe_path(char *buf, int count);                    // 获取当前程序路径
UA_PUBLIC char      *string_dup(const char *str);							// 字符串复制(动态分配)
UA_PUBLIC WORD		calc_check_sum(WORD init_check_sum, BYTE *p_buf, DWORD len, BOOL b_add_high); // 计算校验和
#ifdef __cplusplus
}
#endif

#endif	//_UA_GLOBAL_H

