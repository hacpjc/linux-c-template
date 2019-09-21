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

static void free_node_cb(struct rcu_head *head)
{
	struct threadwq_job *job =
		caa_container_of(head, struct threadwq_job, rcu_head);
	void (*cb_free)(struct threadwq_job *job, void *priv);

	cb_free = job->cb_free;
	cb_free(job, job->priv);
}

static void thread_func(void *in)
{
	struct threadwq *twq = in;

	unsigned long int dequeue_peak = 0, dequeue = 0, sleep = 0, accl = 0;

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

	while (twq->exit == 0)
	{
		struct threadwq_job *job;
		dequeue = 0;

		/*
		 * dequeue job to do
		 */
		job = dequeue_one_job(twq);
		while (job)
		{
			accl++;
			dequeue++;
			job->cb_func(job, job->priv);
			call_rcu(&job->rcu_head, free_node_cb);

			job = dequeue_one_job(twq); // next job
		}

		if (dequeue > dequeue_peak)
		{
			dequeue_peak = dequeue;
		}

		/*
		 * wait for next job
		 */
		sleep++;
#if THREADWQ_BLOCKED_ENQUEUE
		pthread_mutex_lock(&twq->mutex);
		pthread_cond_wait(&twq->cond, &twq->mutex);
		pthread_mutex_unlock(&twq->mutex);
#else
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec = 0;
			ts.tv_nsec = 10 * 100000;

			pthread_mutex_lock(&twq->mutex);
			pthread_cond_timedwait(&twq->cond, &twq->mutex, &ts);
			pthread_mutex_unlock(&twq->mutex);
		}
#endif
	}

	VBS("dequeue peak = %lu sleep %lu accl %lu", dequeue_peak, sleep, accl);

	VBS("twq %p offline", twq);
	if (twq->ops.worker_exit)
	{
		twq->ops.worker_exit(twq, twq->ops.worker_exit_priv);
	}

	twq->exit_ack = 1;
	cmm_smp_mb();

	rcu_unregister_thread();
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

void threadwq_add_job(struct threadwq *twq, struct threadwq_job *job)
{
	BUG_ON(job == NULL);

	rcu_read_lock();
	cds_lfq_enqueue_rcu(&twq->lfq, &job->lfq_node);
	rcu_read_unlock();

	/*
	 * * NOTE: If the worker is not waiting, we might get a latency.
	 */
#if THREADWQ_BLOCKED_ENQUEUE
	pthread_mutex_lock(&twq->mutex);
	pthread_cond_signal(&twq->cond);
	pthread_mutex_unlock(&twq->mutex);
#else
	pthread_cond_signal(&twq->cond);
#endif
	return 0;
}
