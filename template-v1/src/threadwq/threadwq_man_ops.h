
#ifndef SRC_THREADWQ_THREADWQ_MAN_INTERNAL_H_
#define SRC_THREADWQ_THREADWQ_MAN_INTERNAL_H_

struct threadwq_man;
struct threadwq_job;
struct threadwq_man_ops
{
	int (*cb_init)(struct threadwq_man *twq_man);
	void (*cb_exit)(struct threadwq_man *twq_man);

	int (*cb_add_job)(struct threadwq_man *twq_man, struct threadwq_job *job);
};

#define DEFINE_THREADWQ_MAN_OPS(_name, _init, _exit, _add_job) \
	struct threadwq_man_ops _name = { .cb_init = _init, .cb_exit = _exit, .cb_add_job = _add_job }

#define DECLARE_THREADWQ_MAN_OPS(_name) \
	extern struct threadwq_man_ops _name

#endif /* SRC_THREADWQ_THREADWQ_MAN_INTERNAL_H_ */
