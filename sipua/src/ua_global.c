/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : ua_global.c
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : ȫ�ֹ��ú���
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#include "ua_port.h"
#include <time.h>

#include "ua_global.h"

// ��ȡ��ǰ����·��
char *get_exe_path(char *buf, int count)
{
    int     result;
    char    *p_path = NULL;

#ifdef WIN32
	result = GetModuleFileName(NULL, buf, count);
#else
	result = readlink("/proc/self/exe", buf, count);
#endif
    if (result > 0 && result < count)
    {
        buf[result] = '\0';
        p_path = buf;
    }
    return p_path;
}

// �ַ�������(��̬����)
char *string_dup(const char *str)
{
	if (str == NULL)
		return NULL;
	else
	{
		int		len = strlen(str) + 1;
		void	*new_str = malloc(len);

		return (char *)memcpy(new_str, str, len);
	}
}

// ����У���
WORD calc_check_sum(WORD init_check_sum, BYTE *p_buf, DWORD len, BOOL b_add_high)
{
	DWORD   count = (len >> 1);
	PWORD   p_data = (PWORD)p_buf;
	DWORD   check_sum = init_check_sum;
	
    while (count-- > 0)
		check_sum += *p_data++;
	
	if ((len & 0x01) == 0x01)
		check_sum += *((PBYTE)p_data);

    if (b_add_high)
    {
        while ((check_sum >> 16) != 0)
            check_sum = (check_sum & 0xFFFF) + (check_sum >> 16);
    }
	
	return (WORD)check_sum;
}


