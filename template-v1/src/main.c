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

#include "mempool/mempool.h"
#include "threadwq/threadwq.h"

#include <time.h>

static int cb_init(struct threadwq_worker *worker, void *unused)
{
	VBS("init worker %p", worker);
	return 0;
}

static void cb_exit(struct threadwq_worker *worker, void *unused)
{
	VBS("exit worker %p", worker);
}

static unsigned long int wait = 0;
static unsigned long int cnt = 0;
static unsigned int job_sz = 0;

void cb_func(struct threadwq_job *job, void *priv)
{
	/*
	 * Do task
	 */
	cnt++;
}

static struct mempool mp;

void cb_free(struct threadwq_job *job, void *priv)
{
	mempool_free(&mp, job);

	uatomic_dec(&job_sz);
}


static void test_threadwq(void)
{
#define TWQNUM 4
	struct timespec ts, ts_now;
	struct threadwq twq[TWQNUM];

	struct threadwq_ops twq_ops = THREQDWQ_OPS_INITIALIZER(cb_init, NULL, cb_exit, NULL);

	rcu_register_thread();

	{
		unsigned int i;

		for (i = 0; i < TWQNUM; i++)
		{
			threadwq_init(&twq[i]);
			threadwq_set_ops(&twq[i], &twq_ops);
			threadwq_exec(&twq[i]);
		}
	}

	clock_gettime(CLOCK_REALTIME, &ts);

	{

		struct threadwq_job *job;
		const unsigned int max_job = 10;
		unsigned int select_twq = 0;

		mempool_init(&mp, "name", sizeof(*job), 128, NULL, NULL);

		// max = 475656601
		// cca_cpu_relax = 323674062
		// mempool = 148476828
		// malloc  = 270412123
		// uatomic = 254584451
		// mempool + cca_cpu_relax = 136757369
		// mempool + job init      = 105684782
		for (;;)
		{
			select_twq++;
			if (select_twq >= TWQNUM)
			{
				select_twq = 0;
			}

			if (uatomic_read(&job_sz) > max_job)
			{
				clock_gettime(CLOCK_REALTIME, &ts_now);
				if ((ts_now.tv_sec - ts.tv_sec) > 10) break;

				usleep(100);
				wait++;
				continue;
			}

			job = mempool_alloc(&mp);
			if (!job)
			{
				clock_gettime(CLOCK_REALTIME, &ts_now);
				if ((ts_now.tv_sec - ts.tv_sec) > 10) break;

				usleep(100);
				wait++;
				continue;
			}

			uatomic_inc(&job_sz);

			threadwq_job_init(job, cb_func, cb_free, NULL);
			threadwq_add_job(&twq[select_twq], job);

			/*
			 * xxx
			 */
			clock_gettime(CLOCK_REALTIME, &ts_now);
			if ((ts_now.tv_sec - ts.tv_sec) > 10) break;
		}

		printf("cnt=%lu, wait=%lu\n", cnt, wait);
	}

	{
		unsigned int i;

		for (i = 0; i < TWQNUM; i++)
		{
			threadwq_exit(&twq[i]);
		}
	}

	rcu_unregister_thread();

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
