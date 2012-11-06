#include "finsh.h"
#include "shell.h"
#include <rtthread.h>
#include <stdio.h>
#include <conio.h>

struct finsh_shell* shell;
// #define RT_ASSERT(x) 

#ifndef RT_TRUE
#define RT_TRUE 1
#endif

#ifndef RT_FALSE
#define RT_FALSE (!RT_TRUE)
#endif

void finsh_system_init(void);

#if !defined (RT_USING_NEWLIB) && !defined (RT_USING_MINILIBC)
#ifndef _WIN32
int strcmp (const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) s1++, s2++;

	return (*s1 - *s2);
}
#endif

#ifdef RT_USING_HEAP
char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *tmp = (char *)rt_malloc(len);

	if(!tmp) return NULL;

	rt_memcpy(tmp, s, len);
	return tmp;
}
#endif

#if !defined(__CC_ARM) && !defined(__IAR_SYSTEMS_ICC__)
int isalpha( int ch )
{
	return (unsigned int)((ch | 0x20) - 'a') < 26u;
}

#ifndef _WIN32
int atoi(const char* s)
{
	long int v=0;
	int sign=1;
	while ( *s == ' '  ||  (unsigned int)(*s - 9) < 5u) s++;

	switch (*s)
	{
	case '-': sign=-1;
	case '+': ++s;
	}

	while ((unsigned int) (*s - '0') < 10u)
	{
		v=v*10+*s-'0'; ++s;
	}

	return sign==-1?-v:v;
}
#endif

int isprint(unsigned char ch)
{
    return (unsigned int)(ch - ' ') < 127u - ' ';
}
#endif
#endif

/**
 * @ingroup finsh
 *
 * This function set the echo mode of finsh shell.
 *
 * FINSH_OPTION_ECHO=0x01 is echo mode, other values are none-echo mode.
 *
 * @param echo the echo mode
 */
void finsh_set_echo(u_long echo)
{
	shell->echo_mode = (u_char)echo;
}

/**
 * @ingroup finsh
 *
 * This function gets the echo mode of finsh shell.
 *
 * @return the echo mode
 */
u_long finsh_get_echo()
{
	RT_ASSERT(shell != RT_NULL);

	return shell->echo_mode;
}

void finsh_auto_complete(char* prefix)
{
	extern void list_prefix(char* prefix);

	rt_kprintf("\r\n");
	list_prefix(prefix);
	rt_kprintf("%s%s", FINSH_PROMPT, prefix);
}

void finsh_run_line(struct finsh_parser* parser, const char *line)
{
	const char* err_str;

	rt_kprintf("\r\n");
	finsh_parser_run(parser, (unsigned char*)line);

	/* compile node root */
	if (finsh_errno() == 0)
	{
		finsh_compiler_run(parser->root);
	}
	else
	{
		err_str = finsh_error_string(finsh_errno());
		rt_kprintf("%s\r\n", err_str);
	}

	/* run virtual machine */
	if (finsh_errno() == 0)
	{
		char ch;
		finsh_vm_run();

		ch = (unsigned char)finsh_stack_bottom();
		if (ch > 0x20 && ch < 0x7e)
		{
			rt_kprintf("\t'%c', %d, 0x%08x\r\n",
				(unsigned char)finsh_stack_bottom(),
				(unsigned int)finsh_stack_bottom(),
				(unsigned int)finsh_stack_bottom());
		}
		else
		{
			rt_kprintf("\t%d, 0x%08x\r\n",
				(unsigned int)finsh_stack_bottom(),
				(unsigned int)finsh_stack_bottom());
		}
	}

    finsh_flush(parser);
}

#ifdef FINSH_USING_HISTORY
int finsh_handle_history(struct finsh_shell* shell, unsigned char ch)
{
	if (ch == 0xe0)
	{
		shell->stat = WAIT_FUNC_KEY;
		return RT_TRUE;
	}

	/*
	 * handle up and down key
	 * up key  : 0x1b 0x5b 0x41
	 * down key: 0x1b 0x5b 0x42
	 */
	if (ch == 0x1b)
	{
		shell->stat = WAIT_SPEC_KEY;
		return RT_TRUE;
	}

	if ((shell->stat == WAIT_SPEC_KEY))
	{
		if (ch == 0x5b)
		{
			shell->stat = WAIT_FUNC_KEY;
			return RT_TRUE;
		}

		shell->stat = WAIT_NORMAL;
		return RT_FALSE;
	}

	if (shell->stat == WAIT_FUNC_KEY)
	{
		shell->stat = WAIT_NORMAL;

		if (ch == 0x41|| ch == 0x48) /* up key */
		{
			/* prev history */
			if (shell->current_history > 0)shell->current_history --;
			else
			{
				shell->current_history = 0;
				return RT_TRUE;
			}

			/* copy the history command */
			memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
				FINSH_CMD_SIZE);
			shell->line_position = strlen(shell->line);
			shell->use_history = 1;
		}
		else if (ch == 0x42 || ch == 0x50) /* down key */
		{
			/* next history */
			if (shell->current_history < shell->history_count - 1)
				shell->current_history ++;
			else
			{
				/* set to the end of history */
				if (shell->history_count != 0)
				{
					shell->current_history = shell->history_count - 1;
				}
				else return RT_TRUE;
			}

			memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
				FINSH_CMD_SIZE);
			shell->line_position = strlen(shell->line);
			shell->use_history = 1;
		}

		if (shell->use_history)
		{
			rt_kprintf("\033[2K\r");
			rt_kprintf("%s%s", FINSH_PROMPT, shell->line);
			return RT_TRUE;;
		}
	}

	return RT_FALSE;
}

void finsh_push_history(struct finsh_shell* shell)
{
	if ((shell->use_history == 0) && (shell->line_position != 0))
	{
		/* push history */
		if (shell->history_count >= FINSH_HISTORY_LINES)
		{
			/* move history */
			int index;
			for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
			{
				memcpy(&shell->cmd_history[index][0],
					&shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
			}
			memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
			memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

			/* it's the maximum history */
			shell->history_count = FINSH_HISTORY_LINES;
		}
		else
		{
			memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
			memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

			/* increase count and set current history position */
			shell->history_count ++;
		}
	}
	shell->current_history = (u_char)shell->history_count;
}
#endif

#ifndef RT_USING_HEAP
struct finsh_shell _shell;
#endif
void finsh_run(void)
{
	extern u_long UART_GetChar(void);
    char ch;

	finsh_system_init();
	/* normal is echo mode */
	shell->echo_mode = 1;

    finsh_init(&shell->parser);
	rt_kprintf(FINSH_PROMPT);

	while (1)
	{
		/* read one character from device */
		while (1)
		{
			// ch = UART_GetChar();
			ch = _getch();
			
			/* handle history key */
			#ifdef FINSH_USING_HISTORY
			if (finsh_handle_history(shell, ch) == RT_TRUE) continue;
			#endif

			/* handle tab key */
			if (ch == '\t')
			{
				/* auto complete */
				finsh_auto_complete(&shell->line[0]);
				/* re-calculate position */
				shell->line_position = strlen(shell->line);
				continue;
			}
			/* handle backspace key */
			else if (ch == 0x7f || ch == 0x08)
			{
				if (shell->line_position != 0)
				{
					rt_kprintf("%c %c", ch, ch);
				}
				if (shell->line_position <= 0) shell->line_position = 0;
				else shell->line_position --;
				shell->line[shell->line_position] = 0;
				continue;
			}

			/* handle end of line, break */
			if (ch == '\r' || ch == '\n')
			{
				/* change to ';' and break */
				shell->line[shell->line_position] = ';';

				#ifdef FINSH_USING_HISTORY
				finsh_push_history(shell);
				#endif

				if (shell->line_position != 0) finsh_run_line(&shell->parser, shell->line);
				else rt_kprintf("\r\n");

				rt_kprintf(FINSH_PROMPT);
				memset(shell->line, 0, sizeof(shell->line));
				shell->line_position = 0;

				break;
			}

			/* it's a large line, discard it */
			if (shell->line_position >= FINSH_CMD_SIZE) shell->line_position = 0;

			/* normal character */
			shell->line[shell->line_position] = ch; ch = 0;
			if (shell->echo_mode) rt_kprintf("%c", shell->line[shell->line_position]);
			shell->line_position ++;
			shell->use_history = 0; /* it's a new command */
		} /* end of device read */
	}
}

static void finsh_shell_thread_entry(void* parameter)
{
	finsh_run();
}

void finsh_system_function_init(const void* begin, const void* end)
{
	_syscall_table_begin = (struct finsh_syscall*) begin;
	_syscall_table_end = (struct finsh_syscall*) end;
}

void finsh_system_var_init(const void* begin, const void* end)
{
	_sysvar_table_begin = (struct finsh_sysvar*) begin;
	_sysvar_table_end = (struct finsh_sysvar*) end;
}

#if defined(__ICCARM__)               /* for IAR compiler */
  #ifdef FINSH_USING_SYMTAB
    #pragma section="FSymTab"
    #pragma section="VSymTab"
  #endif
#endif

#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of finsh";
__declspec(allocate("FSymTab$a")) const struct finsh_syscall __fsym_begin = 
{
	__fsym_begin_name,
	__fsym_begin_desc,
	NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of finsh";
__declspec(allocate("FSymTab$z")) const struct finsh_syscall __fsym_end = 
{
	__fsym_end_name,
	__fsym_end_desc,
	NULL
};

/*
 * @ingroup finsh
 *
 * This function will initialize finsh shell
 */
void finsh_system_init(void)
{
#ifdef FINSH_USING_SYMTAB
#ifdef __CC_ARM                 /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    extern const int VSymTab$$Base;
    extern const int VSymTab$$Limit;
	finsh_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
	finsh_system_var_init(&VSymTab$$Base, &VSymTab$$Limit);
#elif defined (__ICCARM__)      /* for IAR Compiler */
    finsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
    finsh_system_var_init(__section_begin("VSymTab"),
                          __section_end("VSymTab"));
#elif defined (__GNUC__)        /* GNU GCC Compiler */
	extern const int __fsymtab_start;
	extern const int __fsymtab_end;
	extern const int __vsymtab_start;
	extern const int __vsymtab_end;
	finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
	finsh_system_var_init(&__vsymtab_start, &__vsymtab_end);
#endif
	unsigned int *ptr_begin, *ptr_end;

	ptr_begin = (unsigned int*)&__fsym_begin; ptr_begin += 3;
	while (*ptr_begin == 0) ptr_begin ++;

	ptr_end = (unsigned int*) &__fsym_end; ptr_end --;
	while (*ptr_end == 0) ptr_end --;

	finsh_system_function_init(ptr_begin, ptr_end);
#endif

	/* create or set shell structure */
#ifdef RT_USING_HEAP
	shell = (struct finsh_shell*)rt_malloc(sizeof(struct finsh_shell));
#else
	shell = &_shell;
#endif
	if (shell == NULL)
	{
		rt_kprintf("no memory for shell\n");
		return;
	}

	memset(shell, 0, sizeof(struct finsh_shell));
}

void finsh_thread_init(void)
{
	rt_thread_t tid;
	tid = rt_thread_create("finsh", finsh_shell_thread_entry, RT_NULL,
		4096, 20, 10);
	if (tid != RT_NULL)
	{
		rt_thread_startup(tid);
	}
}
