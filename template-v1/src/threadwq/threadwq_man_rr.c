#include "lgu/lgu.h"

#include "threadwq.h"
#include "threadwq_man.h"
#include "threadwq_man_ops.h"

#include "threadwq_man_rr.h"

/*
 * Assume the job is w/ similar cost. Then round-robin is usually good. Consider others if cache-hit is critical.
 */
static int rr_add_job(struct threadwq_man *man, struct threadwq_job *job)
{
	BUG_ON(man == NULL || job == NULL);

	/*
	 * Round-robin (RR): Add job into twq one by one.
	 */
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
