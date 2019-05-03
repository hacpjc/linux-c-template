
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "initops.h"

static int initops_dummy_init(void *unused)
{
	return 0;
}

static void initops_dummy_exit(void *unused)
{
	return;
}

DEFINE_INITOPS(initops_dummy, INITOPS_ORDER_MAX, initops_dummy_init, initops_dummy_exit, NULL);

/*
 * Create automatically by gcc.
 */
extern struct initops *__start_section_initops;
extern struct initops *__stop_section_initops;

static int initops_exec_exit_by_order(initops_order_t order)
{
	struct initops **iter = &__start_section_initops;

	/*
	 * Find proper order number and execute.
	 */
	for (; iter < &__stop_section_initops; iter++)
	{
		struct initops *initops = *iter;
		if (initops->order == order && initops->exit)
		{
			initops->exit(initops->priv);
		}
	}
}

static int initops_exec_init_by_order(initops_order_t order)
{
	int ret;
	struct initops **iter = &__start_section_initops;

	/*
	 * Find proper order number and execute.
	 */
	for (; iter < &__stop_section_initops; iter++)
	{
		struct initops *initops = *iter;
		if (initops->order == order && initops->init)
		{
			ret = initops->init(initops->priv);
			if (ret < 0)
			{
				goto fallback;
			}
		}
	}

	return 0;

fallback:
	if (iter == &__start_section_initops)
	{
		return -1; // Do not need to do fallback task.
	}

	for (; iter > &__start_section_initops; iter--)
	{
		struct initops *initops = *iter;
		if (initops->order == order && initops->exit)
		{
			initops->exit(initops->priv);
		}
	}

	return -1;
}

void initops_exec_exit(void)
{
	int order;

	for (order = INITOPS_ORDER_MAX; order >= INITOPS_ORDER_FIRST; order--)
	{
		int ret;
		struct initops **iter = &__start_section_initops;

		initops_exec_exit_by_order(order);
	}
}

int initops_exec_init(void)
{
	int order;

	for (order = INITOPS_ORDER_FIRST; order <= INITOPS_ORDER_MAX; order++)
	{
		if (initops_exec_init_by_order(order))
		{
			goto fallback; // Oops. Try to remind successful init.
		}
	}

	return 0;

fallback:
	order -= 1;
	for (; order > INITOPS_ORDER_FIRST /* Skip first! */; order--)
	{
		initops_exec_exit_by_order(order);
	}

	return -1;
}

