#include "finsh.h"
#include <rtthread.h>
#include <stdio.h>

static struct finsh_syscall* _next_syscall(struct finsh_syscall* call)
{
	unsigned int *ptr;
	ptr = (unsigned int*) (call + 1);
	while ((*ptr == 0) && ((unsigned int*)ptr < (unsigned int*) _syscall_table_end))
		ptr ++;

	return (struct finsh_syscall*)ptr;
}

long list(void)
{
	struct finsh_syscall_item* syscall_item;
	struct finsh_sysvar_item*  sysvar_item;

	rt_kprintf("--Function List:\r\n");
	{
		struct finsh_syscall* index;
		for (index = _syscall_table_begin; index < _syscall_table_end; index = _next_syscall(index))
		{
#ifdef FINSH_USING_DESCRIPTION
			rt_kprintf("%-16s -- %s\r\n", index->name, index->desc);
#else
			rt_kprintf("%s\r\n", index->name);
#endif
		}
	}

	/* list syscall list */
	syscall_item = global_syscall_list;
	while (syscall_item != NULL)
	{
		rt_kprintf("[l] %s\r\n", syscall_item->syscall.name);
		syscall_item = syscall_item->next;
	}

	rt_kprintf("--Variable List:\r\n");
	{
		struct finsh_sysvar* index;
		for (index = _sysvar_table_begin; index < _sysvar_table_end; index ++)
		{
#ifdef FINSH_USING_DESCRIPTION
			rt_kprintf("%-16s -- %s\r\n", index->name, index->desc);
#else
			rt_kprintf("%s\r\n", index->name);
#endif
		}
	}

	sysvar_item = global_sysvar_list;
	while (sysvar_item != NULL)
	{
		rt_kprintf("[l] %s\r\n", sysvar_item->sysvar.name);
		sysvar_item = sysvar_item->next;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT(list, list all symbol in system);

static int str_is_prefix(const char* prefix, const char* str)
{
	while ((*prefix) && (*prefix == *str))
	{
		prefix ++;
		str ++;
	}

	if (*prefix == 0) return 0;
	return -1;
}

void list_prefix(char* prefix)
{
	struct finsh_syscall_item* syscall_item;
	struct finsh_sysvar_item*  sysvar_item;
	u_short func_cnt, var_cnt;
	const char* name_ptr;

	func_cnt = 0;
	var_cnt  = 0;
	name_ptr = NULL;

	{
		struct finsh_syscall* index;
		for (index = _syscall_table_begin; index < _syscall_table_end; index = _next_syscall(index))
		{
			if (str_is_prefix(prefix, index->name) == 0)
			{
				if (func_cnt == 0)
					rt_kprintf("--function:\r\n");

				func_cnt ++;
				/* set name_ptr */
				name_ptr = index->name;

#ifdef FINSH_USING_DESCRIPTION
				rt_kprintf("%-16s -- %s\r\n", index->name, index->desc);
#else
				rt_kprintf("%s\r\n", index->name);
#endif
			}
		}
	}

	/* list syscall list */
	syscall_item = global_syscall_list;
	while (syscall_item != NULL)
	{
		if (str_is_prefix(prefix, syscall_item->syscall.name) == 0)
		{
			if (func_cnt == 0)
				rt_kprintf("--function:\r\n");
			func_cnt ++;
			/* set name_ptr */
			name_ptr = syscall_item->syscall.name;

			rt_kprintf("[l] %s\r\n", syscall_item->syscall.name);
		}
		syscall_item = syscall_item->next;
	}

	{
		struct finsh_sysvar* index;
		for (index = _sysvar_table_begin; index < _sysvar_table_end; index ++)
		{
			if (str_is_prefix(prefix, index->name) == 0)
			{
				if (var_cnt == 0)
					rt_kprintf("--variable:\r\n");

				var_cnt ++;
				/* set name ptr */
				name_ptr = index->name;

#ifdef FINSH_USING_DESCRIPTION
				rt_kprintf("%-16s -- %s\r\n", index->name, index->desc);
#else
				rt_kprintf("%s\r\n", index->name);
#endif
			}
		}
	}

	sysvar_item = global_sysvar_list;
	while (sysvar_item != NULL)
	{
		if (str_is_prefix(prefix, sysvar_item->sysvar.name) == 0)
		{
			if (var_cnt == 0)
				rt_kprintf("--variable:\r\n");

			var_cnt ++;
			/* set name ptr */
			name_ptr = sysvar_item->sysvar.name;

			rt_kprintf("[l] %s\r\n", sysvar_item->sysvar.name);
		}
		sysvar_item = sysvar_item->next;
	}

	/* only one matched */
	if ((func_cnt + var_cnt) == 1)
	{
		strncpy(prefix, name_ptr, strlen(name_ptr));
	}
}

#ifdef FINSH_USING_SYMTAB
static int dummy = 0;
FINSH_VAR_EXPORT(dummy, finsh_type_int, dummy variable for finsh);
#endif
