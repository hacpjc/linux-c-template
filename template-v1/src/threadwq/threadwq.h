/*!
 * \file threadwq.h
 * \brief threadwq (thread work queue) is used to deliver jobs into a thread. The caller can wake up
 *     the thread to run anytime.
 */
#ifndef SRC_THREADWQ_THREADWQ_H_
#define SRC_THREADWQ_THREADWQ_H_

#include <pthread.h>

#include <urcu.h>
#include <urcu/list.h>
#include <urcu/rculfqueue.h> // lfq = lock-free queue

#include "lgu/lgu.h"

#include "threadwq/threadwq_man.h"

#define THREADWQ_LFQ_CREATE_RCU_DATA (1) //!< Say 0 to disable rcu thread.

/*
 * Policy:
 * - If this is not critical app, BLOCKED_ENQUEUE = 1
 * - If this is critical, BLOCKED_ENQUEUE = 0
 * - If this is very crital and allow more cpu usage: say ENQUEUE_TIMEDWAIT = 0.
 */
#define THREADWQ_BLOCKED_ENQUEUE (1)
#if !THREADWQ_BLOCKED_ENQUEUE
#define THREADWQ_NONBLOCKED_ENQUEUE (1)
#define THREADWQ_NONBLOCKED_ENQUEUE_TIMEDWAIT (0) //!< 1 = 1 nanosecond, or 0 = cpu_relax
#endif

#define THREADWQ_STAT_PERIOD (1000) //!< Recommend: Use your max job size?

struct threadwq_job
{
	void *priv;
	void (*cb_start)(struct threadwq_job *job, void *priv);
	void (*cb_finish)(struct threadwq_job *job, void *priv);

	struct rcu_head rcu_head;
	struct cds_lfq_node_rcu lfq_node;
};

static inline __attribute__((unused)) inline
void threadwq_job_init(struct threadwq_job *job,
	void (*cb_start)(struct threadwq_job *job, void *priv),
	void (*cb_finish)(struct threadwq_job *job, void *priv),
	void *priv)
{
	job->cb_start = cb_start;
	job->cb_finish = cb_finish;
	job->priv = priv;

	cds_lfq_node_init_rcu(&job->lfq_node);
}

struct threadwq_ops
{
	int (*worker_init)(struct threadwq *twq, void *priv);
	void *worker_init_priv;
	void (*worker_exit)(struct threadwq *twq, void *priv);
	void *worker_exit_priv;
};

#define THREQDWQ_OPS_INITIALIZER(_init, _initpriv, _exit, _exitpriv) \
	{ _init, _initpriv, _exit, _exitpriv }

struct threadwq
{
	unsigned int exit;
	unsigned int exit_ack;
	unsigned int running;

	pthread_t tid;
	pthread_attr_t attr;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	struct cds_lfq_queue_rcu lfq;

	struct threadwq_ops ops;

	unsigned int busy;
};

int threadwq_init(struct threadwq *twq);
int threadwq_init_multi(struct threadwq *twq_tbl, const unsigned int nr);
void threadwq_exit(struct threadwq *twq);
void threadwq_exit_multi(struct threadwq *twq_tbl, const unsigned int nr);
void threadwq_set_ops(struct threadwq *twq, const struct threadwq_ops *ops);
void threadwq_set_ops_multi(struct threadwq *twq_tbl, const struct threadwq_ops *ops, const unsigned int nr);
int threadwq_exec(struct threadwq *twq);
int threadwq_exec_multi(struct threadwq *twq_tbl, const unsigned int nr);

static inline __attribute__((unused))
void threadwq_add_job_locked(struct threadwq *twq, struct threadwq_job *job)
{
	/*
	 * Assume the mutex is acquired by caller.
	 */
	BUG_ON(job == NULL);

	rcu_read_lock();
	cds_lfq_enqueue_rcu(&twq->lfq, &job->lfq_node);
	rcu_read_unlock();

//	pthread_cond_signal(&twq->cond); // CAUTION: Plz send signal at caller, too.
}

static inline __attribute__((unused))
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
#elif THREADWQ_NONBLOCKED_ENQUEUE
	pthread_cond_signal(&twq->cond);
#else
#error "fixme"
#endif
}


#endif /* TEMPLATE_V1_SRC_THREADWQ_THREADWQ_H_ */
