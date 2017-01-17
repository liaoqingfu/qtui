/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : types.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2010/06/01
* Description  : 变量类型头文件
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#if !defined(WIN32)
typedef char                CHAR;
typedef unsigned char       BYTE;
typedef unsigned short int  WORD;
typedef unsigned int        DWORD;
typedef unsigned char       *PBYTE;
typedef char       			*LPCSTR;
typedef unsigned short int  *PWORD;
typedef short int  			*PSHORT;
typedef unsigned int        *PDWORD;
typedef void                VOID;
typedef void                *HANDLE;
typedef int                 INT;

typedef signed char         TCHAR;

typedef unsigned short      WCHAR;
typedef unsigned int        UINT;
typedef unsigned long       ULONG;
typedef long       			LONG;

#define CONST       const
#define STATIC      static

#ifndef NULL
#define NULL        ((void *)0)
#endif

#ifndef BOOL
#define BOOL        int
#endif
#ifndef FALSE
#define FALSE       0
#endif
#ifndef TRUE
#define TRUE        1
#endif

#endif


//typedef enum {FALSE = 0, TRUE = !FALSE} BOOL;

//----------------- 1. 宏及结构本声明 -----------------

//----------------- 2. 变量声明 -----------------------

//----------------- 3. 函数声明 -----------------------


//----------------- 1. 宏及结构本声明 -----------------

//----------------- 2. 变量定义 -----------------------

//----------------- 3. 函数声明 -----------------------

//----------------- 4. 函数定义 -----------------------

#endif  // __TYPE_H__
