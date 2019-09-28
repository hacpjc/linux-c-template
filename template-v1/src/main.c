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

static unsigned int database[16384];

static int cb_init_worker(struct threadwq *twq, void *unused)
{
	VBS("init twq %p", twq);
	memset(database, 0x00, sizeof(database));
	database[16383] = 1;
	return 0;
}

static void cb_exit_worker(struct threadwq *twq, void *unused)
{
	VBS("exit twq %p", twq);
}

static unsigned long int wait = 0; // Queue full counter
static unsigned long int cnt_start = 0;
static unsigned long int cnt_finish = 0;

#include "json-c/json.h"

void cb_start(struct threadwq_job *job, void *priv)
{
	/*
	 * Do task
	 */
	uatomic_inc(&cnt_start);

#if 0 // io-bound job, low cpu cost
	{
		struct json_object *jo = json_object_from_file("/tmp/ramfs/test.jon");

		json_object_put(jo);
	}
#endif

#if 1 // cpu-bound job, high cpu/memory cost... Time to prove the ability.
	{
		unsigned int i;

		for (i = 0; i < 16384; i++)
		{
			if (database[i] == 1)
			{
				break;
			}
		}
	}
#endif
}

static struct mempool mp;

void cb_finish(struct threadwq_job *job, void *priv)
{
	mempool_free(&mp, job);
	uatomic_inc(&cnt_finish);
}

#define TWQNUM 4 // CPU number.
#define TEST_TIME 10 // sec

static void *threadfunc2(void *twqin)
{
	struct timespec ts, ts_now;
	struct threadwq *twq = twqin;

	rcu_register_thread();


	clock_gettime(CLOCK_REALTIME, &ts);

	{

		struct threadwq_job *job;
		unsigned int select_twq = 0;

		// max = 475656601
		// cca_cpu_relax = 323674062
		// mempool = 148476828
		// malloc  = 270412123
		// uatomic = 254584451
		// mempool + cca_cpu_relax = 136757369
		// mempool + job init      = 105684782
		//
		// result: 6757808
		for (;;)
		{
			select_twq++;
			if (select_twq >= TWQNUM)
			{
				select_twq = 0;
			}

			job = mempool_alloc(&mp);
			if (!job)
			{
				clock_gettime(CLOCK_REALTIME, &ts_now);
				if ((ts_now.tv_sec - ts.tv_sec) > TEST_TIME) break;

				caa_cpu_relax();
				wait++;
				continue;
			}

			threadwq_job_init(job, cb_start, cb_finish, NULL);
			threadwq_add_job(&twq[select_twq], job);

			/*
			 * xxx
			 */
			clock_gettime(CLOCK_REALTIME, &ts_now);
			if ((ts_now.tv_sec - ts.tv_sec) > TEST_TIME) break;
		}

		clock_gettime(CLOCK_REALTIME, &ts_now);
		printf("%u thread:\ncnt=%lu, fail=%lu time=%lu\n",
			TWQNUM,
			cnt_start, wait, (ts_now.tv_sec - ts.tv_sec));
	}

	rcu_unregister_thread();
	return NULL;
}


static void test_threadwq2(void)
{
	struct threadwq twq[TWQNUM];
	struct threadwq_ops twq_ops = THREQDWQ_OPS_INITIALIZER(cb_init_worker, NULL, cb_exit_worker, NULL);

	cnt_start = 0;
	cnt_finish = 0;
	wait = 0;

	BUG_ON(threadwq_init_multi(twq, TWQNUM));
	threadwq_set_ops_multi(twq, &twq_ops, TWQNUM);

	BUG_ON(threadwq_exec_multi(twq, TWQNUM));

	BUG_ON(create_all_cpu_call_rcu_data(0));

	{ // Create another writer thread
		pthread_t tid;
		pthread_attr_t tattr;

		pthread_attr_init(&tattr);

		if (pthread_create(&tid, &tattr, &threadfunc2, twq))
		{
			BUG();
		}

		pthread_join(tid, NULL);
	}

	threadwq_exit_multi(twq, TWQNUM);

	cmm_smp_mb();

	free_all_cpu_call_rcu_data(); // Free all pending call rcu
	printf("\t--> cnt=%lu free=%lu, wait=%lu\n", cnt_start, cnt_finish, wait);

	return;
}

static void *threadfunc3(void *twqmanin)
{
	struct timespec ts, ts_now;
	struct threadwq_man *man = twqmanin;

	rcu_register_thread();

	clock_gettime(CLOCK_REALTIME, &ts);

	{

		struct threadwq_job *job;
		unsigned int accl = 0;

		// max = 475656601
		// cca_cpu_relax = 323674062
		// mempool = 148476828
		// malloc  = 270412123
		// uatomic = 254584451
		// mempool + cca_cpu_relax = 136757369
		// mempool + job init      = 105684782
		//
		// result: 6757808
		for (;;)
		{
			job = mempool_alloc(&mp);
			if (!job)
			{
				clock_gettime(CLOCK_REALTIME, &ts_now);
				if ((ts_now.tv_sec - ts.tv_sec) > TEST_TIME) break;

				caa_cpu_relax();
				wait++; // It means queue full.
				continue;
			}

			accl++;

			threadwq_job_init(job, cb_start, cb_finish, NULL);
			BUG_ON(threadwq_man_add_job(man, job));

			/*
			 * xxx
			 */
			clock_gettime(CLOCK_REALTIME, &ts_now);
			if ((ts_now.tv_sec - ts.tv_sec) > TEST_TIME) break;
		}

		clock_gettime(CLOCK_REALTIME, &ts_now);
		printf("%u thread:\ncnt=%lu, expect=%u, fail=%lu time=%lu\n",
			TWQNUM,
			cnt_start, accl, wait, (ts_now.tv_sec - ts.tv_sec));
	}

	rcu_unregister_thread();

	return NULL;
}

static void test_threadwq3(void)
{
	struct threadwq twq[TWQNUM];
	struct threadwq_ops twq_ops = THREQDWQ_OPS_INITIALIZER(cb_init_worker, NULL, cb_exit_worker, NULL);
	struct threadwq_man twq_man;

	cnt_start = 0;
	cnt_finish = 0;
	wait = 0;

	BUG_ON(threadwq_init_multi(twq, TWQNUM));
	threadwq_set_ops_multi(twq, &twq_ops, TWQNUM);

	BUG_ON(threadwq_exec_multi(twq, TWQNUM));


	BUG_ON(threadwq_man_init(&twq_man, twq, TWQNUM, &threadwq_man_ops_rr4idle));


	BUG_ON(create_all_cpu_call_rcu_data(0));

	{ // Create another writer thread
		pthread_t tid;
		pthread_attr_t tattr;

		pthread_attr_init(&tattr);

		if (pthread_create(&tid, &tattr, &threadfunc3, &twq_man))
		{
			BUG();
		}

		pthread_join(tid, NULL);
	}

	threadwq_exit_multi(twq, TWQNUM);

	threadwq_man_exit(&twq_man);

	cmm_smp_mb();

	free_all_cpu_call_rcu_data();
	printf("\t--> cnt=%lu free=%lu, wait=%lu\n", cnt_start, cnt_finish, wait);

	return;
}

static void test_threadwq(void)
{
	struct timespec ts, ts_now;

	cnt_start = 0;
	cnt_finish = 0;
	wait = 0;

	clock_gettime(CLOCK_REALTIME, &ts);

	for (;;)
	{
		cb_start(NULL, NULL);

		/*
		 * xxx
		 */
		clock_gettime(CLOCK_REALTIME, &ts_now);
		if ((ts_now.tv_sec - ts.tv_sec) > TEST_TIME) break;
	}

	clock_gettime(CLOCK_REALTIME, &ts_now);
	printf("no thread:\ncnt=%lu, wait/waste=%lu time=%lu\n", cnt_start, wait, (ts_now.tv_sec - ts.tv_sec));

	printf("\t--> cnt=%lu free=%lu, wait=%lu\n", cnt_start, cnt_finish, wait);
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

	mempool_init(&mp, "name", sizeof(struct threadwq_job), 65536 * 4, NULL, NULL);
	test_threadwq();
	test_threadwq2();
	test_threadwq3();
	mempool_exit(&mp);


	return 0;
}
