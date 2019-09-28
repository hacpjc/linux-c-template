
#ifndef SRC_THREADWQ_THREADWQ_MANAGER_H_
#define SRC_THREADWQ_THREADWQ_MANAGER_H_

#include "threadwq/threadwq_man_ops.h"

struct threadwq;
struct threadwq_man
{
	struct threadwq *twq_pool;
	unsigned int twq_pool_nr;

	const struct threadwq_man_ops *ops;

	/*
	 * per-algorithm private data
	 */
	union
	{
		struct
		{
			unsigned int twq_idx;
		} rr;
	};
};

#include "threadwq_man_rr.h"
#define threadwq_man_ops_default threadwq_man_ops_rr

static inline __attribute__((unused))
int threadwq_man_add_job(struct threadwq_man *man, struct threadwq_job *job)
{
	BUG_ON(job == NULL);

	if (man)
	{
		return man->ops->cb_add_job(man, job);
	}

	/*
	 * Use default manager!
	 */
	printf("...default\n");
	return threadwq_man_ops_default.cb_add_job(man, job);
}

int threadwq_man_init(struct threadwq_man *man,
	struct threadwq *twq_tbl, const unsigned int twq_tbl_nr,
	const struct threadwq_man_ops *ops);
void threadwq_man_exit(struct threadwq_man *man);

#endif /* SRC_THREADWQ_THREADWQ_MANAGER_H_ */
