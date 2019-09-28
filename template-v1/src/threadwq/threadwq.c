#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <urcu.h>
#include <urcu/list.h>


#include <sched.h>

#include "lgu/lgu.h"
#include "threadwq.h"

static unsigned int cpu_affinities[64];
static unsigned int next_aff = 0;
static int use_affinity = 0;

pthread_mutex_t affinity_mutex = PTHREAD_MUTEX_INITIALIZER;

void threadwq_set_ops(struct threadwq *twq, const struct threadwq_ops *ops)
{
	BUG_ON(ops->worker_exit == NULL || ops->worker_init == NULL);
	BUG_ON(sizeof(twq->ops) != sizeof(*ops));
	memcpy(&twq->ops, ops, sizeof(*ops));
}

void threadwq_set_ops_multi(struct threadwq *twq_tbl, const struct threadwq_ops *ops, const unsigned int nr)
{
	unsigned int i;
	struct threadwq *twq;

	for (i = 0; i < nr; i++)
	{
		twq = &twq_tbl[i];
		threadwq_set_ops(twq, ops);
	}
}

int threadwq_init(struct threadwq *twq)
{
	twq->exit = 0;
	twq->exit_ack = 0;
	twq->running = 0;

	pthread_cond_init(&twq->cond, NULL);
	pthread_mutex_init(&twq->mutex, NULL);

	{
		cds_lfq_init_rcu(&twq->lfq, call_rcu);
#if THREADWQ_LFQ_CREATE_RCU_DATA
		if (create_all_cpu_call_rcu_data(0))
		{
			/* Optional, but we should be warned. */
			ERR("Per-CPU call_rcu() worker threads unavailable. Using default global worker thread.");
		}
#endif
	}

	memset(&twq->ops, 0x00, sizeof(twq->ops));

	return 0;
}

int threadwq_init_multi(struct threadwq *twq_tbl, const unsigned int nr)
{
	unsigned int i;
	struct threadwq *twq;

	for (i = 0; i < nr; i++)
	{
		twq = &twq_tbl[i];

		if (threadwq_init(twq))
		{
			threadwq_exit_multi(twq_tbl, i);
			return (nr - i) * (-1);
		}
	}

	return 0; // ok
}

static void kill_online_worker(struct threadwq *twq)
{
	VBS("Push worker to offline");

	/*
	 * Wait all workers to exit.
	 */
	while (1) // Pooling until all workers finish the job.
	{
		pthread_mutex_lock(&twq->mutex);
		twq->exit++;
		pthread_cond_broadcast(&twq->cond);
		pthread_mutex_unlock(&twq->mutex);

		cmm_smp_mb();
		if (twq->exit_ack)
		{
			break;
		}
	}

	pthread_join(twq->tid, NULL);
}

void threadwq_exit(struct threadwq *twq)
{
	/*
	 * Warn the user if queue is not empty. Possibly forget to flush queue first.
	 */

	kill_online_worker(twq);
}

void threadwq_exit_multi(struct threadwq *twq_tbl, const unsigned int nr)
{
	unsigned int i;
	struct threadwq *twq;

	for (i = 0; i < nr; i++)
	{
		twq = &twq_tbl[i];
		threadwq_exit(twq);
	}
}

static inline struct threadwq_job *dequeue_one_job(struct threadwq *twq)
{
	struct threadwq_job *job;
	struct cds_lfq_node_rcu *lfq_node;

	rcu_read_lock();
	lfq_node = cds_lfq_dequeue_rcu(&twq->lfq);
	rcu_read_unlock();

	if (!lfq_node)
	{
		return NULL;
	}

	job = caa_container_of(lfq_node, struct threadwq_job, lfq_node);
	return job;
}

static void __exec_finish_rcu(struct rcu_head *head)
{
	struct threadwq_job *job =
		caa_container_of(head, struct threadwq_job, rcu_head);
	void (*cb_finish)(struct threadwq_job *job, void *priv);

	cb_finish = job->cb_finish;
	cb_finish(job, job->priv);
}


#if THREADWQ_BLOCKED_ENQUEUE
#define wait4job(_twq) \
	{ \
		pthread_mutex_lock(&twq->mutex); \
		pthread_cond_wait(&twq->cond, &twq->mutex); \
		pthread_mutex_unlock(&twq->mutex); \
	}
#elif THREADWQ_NONBLOCKED_ENQUEUE
#if THREADWQ_NONBLOCKED_ENQUEUE_TIMEDWAIT
#define wait4job(_twq) \
	{ \
		struct timespec ts; \
		clock_gettime(CLOCK_REALTIME, &ts); \
		ts.tv_sec = 0; \
		ts.tv_nsec = THREADWQ_NONBLOCKED_ENQUEUE_TIMEDWAIT; \
		pthread_mutex_lock(&twq->mutex); \
		pthread_cond_wait(&twq->cond, &twq->mutex); \
		pthread_mutex_unlock(&twq->mutex); \
	}
#else
#define wait4job(_twq) caa_cpu_relax();
#endif
#else
#error "fixme"
#endif // THREADWQ_BLOCKED_ENQUEUE

static void *thread_func(void *in)
{
	struct threadwq *twq = in;

	rcu_register_thread();

	VBS("twq %p online", twq);
	if (twq->ops.worker_init)
	{
		/* Basic hint: sched_setaffinity & rcu_register_thread. */
		if (twq->ops.worker_init(twq, twq->ops.worker_init_priv))
		{
			ERR("Failed to exec user initializer");
			BUG(); // FIXME: Hard to recover...
		}
	}

	twq->running = 1;
	cmm_smp_mb();

	{
		unsigned int accl = 0, idle = 0;

		while (caa_unlikely(twq->exit == 0))
		{
			struct threadwq_job *job;

			accl++;
			if (caa_unlikely(accl >= THREADWQ_STAT_PERIOD))
			{
				uatomic_set(&twq->busy, accl - idle);
				accl = 0;
				idle >>= 1;
			}

			/*
			 * Dequeue & execute job.
			 */
			job = dequeue_one_job(twq);
			if (caa_unlikely(!job))
			{
				idle++;
				wait4job(twq);
				continue; // no job
			}

			do
			{
				job->cb_start(job, job->priv);
				call_rcu(&job->rcu_head, __exec_finish_rcu);

				job = dequeue_one_job(twq); // next job
			} while (job);

			wait4job(twq);
		}
	}

	VBS("twq %p offline", twq);
	if (twq->ops.worker_exit)
	{
		twq->ops.worker_exit(twq, twq->ops.worker_exit_priv);
	}

	twq->exit_ack = 1;
	cmm_smp_mb();

	rcu_unregister_thread();

	return NULL;
}

int threadwq_exec(struct threadwq *twq)
{
	pthread_attr_init(&(twq->attr));
	pthread_attr_setdetachstate(&(twq->attr), PTHREAD_CREATE_JOINABLE);

	if (pthread_create(&twq->tid, &(twq->attr), &thread_func, (void *) twq))
	{
		ERR("Cannot create pthread %s", strerror(errno));
		return -1;
	}

	while (1)
	{
		if (twq->running) break;

		caa_cpu_relax();
	}

	return 0; // ok
}

int threadwq_exec_multi(struct threadwq *twq_tbl, const unsigned int nr)
{
	unsigned int i;
	struct threadwq *twq;

	for (i = 0; i < nr; i++)
	{
		twq = &twq_tbl[i];

		if (threadwq_exec(twq))
		{
			return (nr - i) * (-1);
		}
	}

	return 0; // ok
}

