
#ifndef SRC_INITOPS_INITOPS_H_
#define SRC_INITOPS_INITOPS_H_

/*!
 * \file initops.h
 * \brief Implement a init/exit hook to avoid having a lot of init tables.
 *
 * \code
static int my_init_func(void *priv)
{
	return 0; // init ok
}

static void my_exit_func(void *priv)
{
	// do exit task.
}

DEFINE_INITOPS(my_initops, INITOPS_ORDER_ANY, my_init_func, my_exit_func, NULL);
 * \endcode
 */


typedef enum
{
	INITOPS_ORDER_FIRST = 0,

	/*
	 * Plz customize the order here.
	 */
	INITOPS_ORDER_XXX,

#define INITOPS_ORDER_ANY (INITOPS_ORDER_LAST)
	INITOPS_ORDER_LAST,
	INITOPS_ORDER_MAX
} initops_order_t;

struct initops
{
	const char *name;
	initops_order_t order;

	int (*init)(void *priv);
	void (*exit)(void *priv);

	void *priv; //!< User's private data to pass to each func.
};

#define INITOPS_INITIALIZER(__name, __order, __init, __exit, __priv) \
	{ .name = __name, .order = __order, .init = __init, .exit = __exit, .priv = __priv }

#define DEFINE_INITOPS(_name, _order, _init, _exit, _priv) \
	struct initops __initops_##_name = INITOPS_INITIALIZER(#_name, _order, _init, _exit, _priv); \
	struct initops * __initops_##_name##_p __attribute__((section ("section_initops"))) = &(__initops_##_name)

extern void initops_exec_exit(void);
extern int initops_exec_init(void);

#endif /* SRC_INITOPS_INITOPS_H_ */
