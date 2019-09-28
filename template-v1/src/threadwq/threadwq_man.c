#include "lgu/lgu.h"

#include "threadwq.h"
#include "threadwq_man.h"

int threadwq_man_init(struct threadwq_man *man,
	struct threadwq *twq_tbl, const unsigned int twq_tbl_nr,
	const struct threadwq_man_ops *ops)
{
	BUG_ON(man == NULL || ops == NULL);
	man->ops = ops;

	BUG_ON(twq_tbl == NULL || twq_tbl_nr <= 0);
	man->twq_pool = twq_tbl;
	man->twq_pool_nr = twq_tbl_nr;

	return man->ops->cb_init(man);
}

void threadwq_man_exit(struct threadwq_man *man)
{
	BUG_ON(man == NULL);
	man->ops->cb_exit(man);
}
