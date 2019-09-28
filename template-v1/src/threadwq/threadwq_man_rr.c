#include "lgu/lgu.h"

#include "threadwq.h"
#include "threadwq_man.h"
#include "threadwq_man_ops.h"

#include "threadwq_man_rr.h"

/*
 * Round-robin (RR): Add job into twq one by one.
 *
 * Assume the job is w/ similar cost. Then round-robin is usually good. Consider others if cache-hit is critical.
 */
static int rr_add_job(struct threadwq_man *man, struct threadwq_job *job)
{
	{
		unsigned int idx = uatomic_read(&man->rr.twq_idx);

		if (idx >= man->twq_pool_nr)
		{
			idx = 0;
			uatomic_set(&man->rr.twq_idx, 0);
		}
		else
		{
			uatomic_inc(&man->rr.twq_idx);
		}

		{
			struct threadwq *twq = &man->twq_pool[idx];
			threadwq_add_job(twq, job);
		}
	}

	return 0;
}

static int rr_init(struct threadwq_man *man)
{
#define my_priv man->rr
	my_priv.twq_idx = 0;
	return 0;
}

static void rr_exit(struct threadwq_man *man)
{
	return;
}

DEFINE_THREADWQ_MAN_OPS(threadwq_man_ops_rr, rr_init, rr_exit, rr_add_job);

#if THREADWQ_BLOCKED_ENQUEUE
/*
 * Round-robin (RR): Check q one by one, and make sure it's in idle state. Good for few writer.
 *
 * Pros: This can make sure the twq won't get too many tasks in queue.
 * Cons: But if you have multiple-writer, one writer may get longer latency (starvation)
 */
static int rr4idle_add_job(struct threadwq_man *man, struct threadwq_job *job)
{
	register unsigned int idx = uatomic_read(&man->rr.twq_idx);

	for (;; idx++)
	{
		struct threadwq *twq;

		if (caa_unlikely(idx >= man->twq_pool_nr))
		{
			idx = 0;
		}

		twq = &man->twq_pool[idx];

		if (caa_likely(pthread_mutex_trylock(&twq->mutex)))
		{
			continue; // lock fail.
		}
		else
		{
			uatomic_set(&man->rr.twq_idx, idx + 1); // Avoid using this index again.

			threadwq_add_job_locked(twq, job);

			pthread_cond_signal(&twq->cond);
			pthread_mutex_unlock(&twq->mutex);
			break;
		}
	} // end for

	return 0;
}

DEFINE_THREADWQ_MAN_OPS(threadwq_man_ops_rr4idle, rr_init, rr_exit, rr4idle_add_job);
#endif
