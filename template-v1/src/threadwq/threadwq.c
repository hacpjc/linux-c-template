#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <urcu.h>
#include <urcu/list.h>

#include "lgu/lgu.h"
#include "threadwq.h"

#define for_each_worker(_twq, _iter, _worker) \
	for (_iter = 0, _worker = &((_twq)->worker_tbl[_iter]); _iter < _twq->worker_nr; _iter++, _worker = &((_twq)->worker_tbl[_iter]))

static void worker_debug(struct threadwq_worker *worker)
{
	INFO("ident: %p -> %p", worker, worker->twq);
	INFO("active: %u", worker->active);
	INFO("exit: %u", worker->exit);
	INFO("queue max: %u", worker->queue_max);
}

static inline struct threadwq_job *worker_dequeue_job(struct threadwq_worker *worker)
{
	return NULL;
}

static void worker_func(void *in)
{
	struct threadwq_worker *worker = in;
	struct threadwq *twq = worker->twq;

	VBS("worker %p online", worker);
	worker_debug(worker);

	if (twq->ops.worker_init)
	{
		/*
		 * Basic hint: sched_setaffinity & rcu_register_thread.
		 */
		if (twq->ops.worker_init(worker, twq->ops.worker_init_priv))
		{
			ERR("Failed to exec user initializer");
			BUG(); // FIXME: Hard to recover...
		}
	}

	while (worker->exit == 0)
	{
		struct threadwq_job *job;

		/*
		 * dequeue job to do
		 */
		job = worker_dequeue_job(worker);
		if (job)
		{
			VBS("Get a job");
		}

		/*
		 * wait for next job
		 */
		pthread_mutex_lock(&worker->mutex);
		pthread_cond_wait(&worker->twq->cond, &worker->mutex);
		pthread_mutex_unlock(&worker->mutex);
	}

	VBS("worker %p offline", worker);
	if (twq->ops.worker_exit)
	{
		twq->ops.worker_exit(worker, twq->ops.worker_exit_priv);
	}
}

static int worker_getonline(struct threadwq_worker *worker)
{
	pthread_attr_init(&(worker->attr));
	pthread_attr_setdetachstate(&(worker->attr), PTHREAD_CREATE_JOINABLE);

	worker->active = 1;
	if (pthread_create(&worker->tid, &(worker->attr), &worker_func, (void *) worker))
	{
		ERR("Cannot create pthread %s", strerror(errno));
		worker->active = 0;
		return -1;
	}

	return 0;
}

static void kill_online_workers(struct threadwq *twq)
{
	VBS("Push all workers to offline");

	/*
	 * Wait all workers to exit.
	 */
	{
		struct threadwq_worker *worker, *save;

		cds_list_for_each_entry_safe(worker, save, &(twq->list), list)
		{
			pthread_mutex_lock(&worker->mutex);
			worker->exit++;
			cmm_smp_mb();
			pthread_cond_broadcast(&twq->cond);
			pthread_mutex_unlock(&worker->mutex);

			pthread_join(worker->tid, NULL);

			worker->active = 0;
			cds_list_del_init(&(worker->list));
		}
	}
}


static int worker_init(struct threadwq_worker *worker, const unsigned int worker_queue_max, struct threadwq *twq)
{
	worker->active = 0;
	worker->exit = 0;

	CDS_INIT_LIST_HEAD(&worker->list);

	BUG_ON(worker_queue_max == 0);
	worker->queue_max = worker_queue_max;
	worker->queue = malloc(sizeof(*(worker->queue)) * worker->queue_max);
	if (!worker->queue)
	{
		ERR("Cannot alloc worker mem with queue sz %u", worker_queue_max);
		return -1;
	}

	worker->twq = twq;

	pthread_mutex_init(&worker->mutex, NULL);

	return 0;
}

static void worker_exit(struct threadwq_worker *worker)
{
	if (worker->queue)
	{
		free(worker->queue);
		worker->queue = NULL;
	}
}

void threadwq_set_ops(struct threadwq *twq, const struct threadwq_ops *ops)
{
	BUG_ON(sizeof(twq->ops) != sizeof(*ops));
	memcpy(&twq->ops, ops, sizeof(*ops));
}

int threadwq_init(struct threadwq *twq)
{
	twq->worker_nr = 0;
	twq->worker_tbl = NULL;

	pthread_cond_init(&twq->cond, NULL);

	CDS_INIT_LIST_HEAD(&twq->list);

	memset(&twq->ops, 0x00, sizeof(twq->ops));

	return 0;
}

void threadwq_exit(struct threadwq *twq)
{
	kill_online_workers(twq);

	{
		unsigned int i;
		struct threadwq_worker *worker;

		for_each_worker(twq, i, worker)
		{
			worker_exit(worker);
		}
	}

	free(twq->worker_tbl);
	twq->worker_tbl = NULL;
	twq->worker_nr = 0;
}

/*
 * Create thread workers. Cannot change twq cfg afterwards.
 */
int threadwq_setup(struct threadwq *twq, const unsigned int worker_nr, const unsigned int worker_queue_max)
{
	twq->worker_nr = worker_nr;
	if (twq->worker_nr == 0)
	{
		return 0; /* Allow empty worker. Can't do any job later, though. */
	}

	twq->worker_tbl = malloc(sizeof(*twq->worker_tbl) * twq->worker_nr);
	if (!twq->worker_tbl)
	{
		ERR("Cannot alloc threadwq mem while worker num=%u", twq->worker_nr);
		return -1;
	}

	memset(twq->worker_tbl, 0x00, sizeof(*twq->worker_tbl) * twq->worker_nr);

	{
		unsigned int i;
		struct threadwq_worker *worker;

		for_each_worker(twq, i, worker)
		{
			if (worker_init(worker, worker_queue_max, twq))
			{
				goto error_worker;
			}
		}

		for_each_worker(twq, i, worker)
		{
			if (worker_getonline(worker))
			{
				goto error_worker;
			}

			cds_list_add(&(worker->list), &twq->list);
		}
	}

	return 0;

error_worker:
	return -1; // Remember to call exit
}


int threadwq_add_job(struct threadwq *twq, struct threadwq_job *job)
{
	pthread_cond_signal(&twq->cond);

	return 0;
}
