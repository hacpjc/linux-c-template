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

struct threadwq_job
{
#define THREADWQ_JOB_PRIV_MAX 4
	void *priv[THREADWQ_JOB_PRIV_MAX];
	void (*func)(struct theadwq_job *job, void **priv);
};

struct threadwq_worker
{
	unsigned int active;
	unsigned int exit;

	pthread_t tid;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	struct threadwq *twq;
	struct cds_list_head list;

	/*
	 * job queue
	 */
	unsigned int queue_max;
	struct threadwq_job *queue;
};

struct threadwq_ops
{
	int (*worker_init)(struct threadwq_worker *worker, void *priv);
	void *worker_init_priv;
	void (*worker_exit)(struct threadwq_worker *worker, void *priv);
	void *worker_exit_priv;
};

#define THREQDWQ_OPS_INITIALIZER(_init, _initpriv, _exit, _exitpriv) \
	{ _init, _initpriv, _exit, _exitpriv }

struct threadwq
{
	unsigned int worker_nr;
	struct threadwq_worker *worker_tbl;

	pthread_cond_t cond;
	struct cds_list_head list;

	struct threadwq_ops ops;
};

void threadwq_set_ops(struct threadwq *twq, const struct threadwq_ops *ops);

int threadwq_add_job(struct threadwq *twq, struct threadwq_job *job);
int threadwq_init(struct threadwq *twq);
void threadwq_exit(struct threadwq *twq);
int threadwq_setup(struct threadwq *twq, const unsigned int worker_nr, const unsigned int worker_queue_max);

#endif /* TEMPLATE_V1_SRC_THREADWQ_THREADWQ_H_ */
