/*
 * File      : symbol.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-03-22     Bernard      first version
 */
#include "finsh.h"

long hello(void);
long version(void);
long list(void);

#ifdef FINSH_USING_SYMTAB
struct finsh_syscall *_syscall_table_begin 	= NULL;
struct finsh_syscall *_syscall_table_end 	= NULL;
struct finsh_sysvar *_sysvar_table_begin 	= NULL;
struct finsh_sysvar *_sysvar_table_end 		= NULL;
#else
struct finsh_syscall _syscall_table[] =
{
	{"list", list},
};
struct finsh_syscall *_syscall_table_begin = &_syscall_table[0];
struct finsh_syscall *_syscall_table_end   = &_syscall_table[sizeof(_syscall_table) / sizeof(struct finsh_syscall)];

struct finsh_sysvar *_sysvar_table_begin  = NULL;
struct finsh_sysvar *_sysvar_table_end    = NULL;
#endif
