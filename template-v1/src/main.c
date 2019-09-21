#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <getopt.h>

#include <urcu.h>

#include "generated/autoconf.h"
#include "lgu/lgu.h"
#include "initops/initops.h"

#include "threadwq/threadwq.h"

static int cb_init(struct threadwq_worker *worker, void *unused)
{
	VBS("init worker %p", worker);
	rcu_register_thread();
	return 0;
}

static void cb_exit(struct threadwq_worker *worker, void *unused)
{
	VBS("exit worker %p", worker);
	rcu_unregister_thread();
}

static void test_threadwq(void)
{
	struct threadwq twq;

	struct threadwq_ops twq_ops = THREQDWQ_OPS_INITIALIZER(cb_init, NULL, cb_exit, NULL);

	threadwq_init(&twq);

	BUG_ON(threadwq_setup(&twq, 2, 16));
	threadwq_set_ops(&twq, &twq_ops);

	{
		unsigned int i;

		for (i = 0; i < 10; i++)
		{
			threadwq_add_job(&twq, &i);
			usleep(100000);
		}
	}

	threadwq_exit(&twq);

	return;
}

static void print_help(const char *path)
{
	printf("%s [--help|-h]\n", path);
	printf("\n");
}

static unsigned int opt_background = 0;

static int argparse(int argc, char **argv)
{
	int c;
	int digit_optind = 0;

	while (1)
	{
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] =
		{
			{ "verbose", no_argument, 0, 'v' },
			{ "quiet", no_argument, 0, 'q' },
			{ "background", no_argument, 0, 'b' },
			{ "help", no_argument, 0, 'h' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "bhvqc:e:",
			long_options, &option_index);
		if (c == -1)
			break;

		switch (c)
		{
		case 'b': // background
			opt_background = 1;
			break;

		case 'q':
			stdmsg_lv_dec();
			break;
		case 'v':
			stdmsg_lv_inc();
			break;
		case 'h':
			print_help(argv[0]);
			return -1;
		default:
			print_help(argv[0]);
			return -1;
		}
	}

	if (optind < argc)
	{
		fprintf(stderr, "non-option ARGV-elements: ");
		while (optind < argc)
			fprintf(stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");
		print_help(argv[0]);
		return -1;
	}

	return 0;
}

static int process_lock_fd = -1;

static int trylock_process(void)
{
	const char *process_lock_path = CONFIG_TEMPLATE_LOCK_PATH;

	process_lock_fd = fio_trylock(process_lock_path);
	if (process_lock_fd < 0)
	{
		ERR("Another instance is locking '%s'", process_lock_path);
		return -1;
	}

	return 0; // ok
}

static void unlock_process(void)
{
	if (process_lock_fd >= 0)
	{
		fio_unlock(process_lock_fd);
		process_lock_fd = -1;
	}
}

static int keep_single(void)
{
	if (trylock_process())
	{
		return -1;
	}

	atexit(unlock_process);
	return 0;
}

int main(int argc, char **argv)
{
	if (argparse(argc, argv))
	{
		return -1;
	}

	/*
	 * Avoid multiple instance. Or my wife will be angry.
	 */
	if (keep_single())
	{
		return -1;
	}

	/*
	 * Run bg
	 */
	if (opt_background)
	{
		DBG("Run as background service");

		if (setsid() < 0)
		{
			ERR("Cannot setsid '%s'", strerror(errno));
			return -1;
		}

		if (daemon(0, 0))
		{
			ERR("Cannot put into background '%s'", strerror(errno));
			return -1;
		}

		stdmsg_lv_set(STDMSG_LV_NONE); // TODO: Use other logger instead.
	}

	/*
	 * Prepare subsys. Init tasks are done here.
	 */
	if (initops_exec_init())
	{
		return -1;
	}

	DBG("Running program: %s", argv[0]);

	test_threadwq();

	return 0;
}
