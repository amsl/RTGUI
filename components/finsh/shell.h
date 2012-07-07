#ifndef __SHELL_H__
#define __SHELL_H__

#include "finsh.h"

#define FINSH_USING_HISTORY
#ifndef FINSH_THREAD_PRIORITY
#define FINSH_THREAD_PRIORITY 20
#endif
#ifndef FINSH_THREAD_STACK_SIZE
#define FINSH_THREAD_STACK_SIZE 2048
#endif
#define FINSH_CMD_SIZE		80

#define FINSH_OPTION_ECHO	0x01
#if defined(RT_USING_DFS) && defined(DFS_USING_WORKDIR)
#define FINSH_PROMPT		finsh_get_prompt()
const char* finsh_get_prompt(void);
#else
#define FINSH_PROMPT		"finsh>>"
#endif

#ifdef FINSH_USING_HISTORY
enum input_stat
{
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};
	#ifndef FINSH_HISTORY_LINES
		#define FINSH_HISTORY_LINES 5
	#endif
#endif

struct finsh_shell
{
	enum input_stat stat;

	u_char echo_mode:1;
	u_char use_history:1;

#ifdef FINSH_USING_HISTORY
	u_char current_history;
	u_short history_count;

	char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];
#endif

	struct finsh_parser parser;

	char line[FINSH_CMD_SIZE];
	u_char line_position;
};

void finsh_set_echo(u_long echo);
u_long finsh_get_echo(void);

void finsh_set_device(const char* device_name);
const char* finsh_get_device(void);

#endif
