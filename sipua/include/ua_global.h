/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : ua_global.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : ȫ�ֹ��ú���
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef _UA_GLOBAL_H
#define _UA_GLOBAL_H

#include <signal.h>

#include "types.h"

//----------------- 1. �꼰�ṹ������ -----------------

//----------------- 2. �������� -----------------------

//----------------- 3. �������� -----------------------
#ifdef __cplusplus
extern "C"
{
#endif
UA_PUBLIC char      *get_exe_path(char *buf, int count);                    // ��ȡ��ǰ����·��
UA_PUBLIC char      *string_dup(const char *str);							// �ַ�������(��̬����)
UA_PUBLIC WORD		calc_check_sum(WORD init_check_sum, BYTE *p_buf, DWORD len, BOOL b_add_high); // ����У���
#ifdef __cplusplus
}
#endif

#endif	//_UA_GLOBAL_H

