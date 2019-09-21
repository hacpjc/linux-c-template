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

#define THREADWQ_LFQ_CREATE_RCU_DATA (1) //!< Say 0 to disable rcu thread.
#define THREADWQ_BLOCKED_ENQUEUE (1)

struct threadwq_job
{
	void *priv;
	void (*cb_func)(struct threadwq_job *job, void *priv);
	void (*cb_free)(struct threadwq_job *job, void *priv);

	struct rcu_head rcu_head;
	struct cds_lfq_node_rcu lfq_node;
};

static inline __attribute__((unused)) inline
void threadwq_job_init(struct threadwq_job *job,
	void (*cb_func)(struct threadwq_job *job, void *priv),
	void (*cb_free)(struct threadwq_job *job, void *priv),
	void *priv)
{
	job->cb_func = cb_func;
	job->cb_free = cb_free;
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

	/*
	 * stat
	 */
};

int threadwq_init(struct threadwq *twq);
void threadwq_exit(struct threadwq *twq);
void threadwq_set_ops(struct threadwq *twq, const struct threadwq_ops *ops);

/*
 * NOTE: Plz avoid infinitely adding job at caller side.
 */
void threadwq_add_job(struct threadwq *twq, struct threadwq_job *job);

#endif /* TEMPLATE_V1_SRC_THREADWQ_THREADWQ_H_ */
