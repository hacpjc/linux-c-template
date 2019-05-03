#ifndef SRC_MEMPOOL_MEMPOOL_H_
#define SRC_MEMPOOL_MEMPOOL_H_

/*!
 * @file mempool.h
 * @brief Implement a kmem-cache-like memory pool.
 */

#include <stdint.h>
#include <pthread.h>
#include "lgu/lgu.h"

struct mempool
{
	unsigned int magic; //!< A magic num for debug purpose.
	unsigned int sz; //!< Slice size.

#define MEMPOOL_NAME_MAX (15 + 1)
	char name[MEMPOOL_NAME_MAX]; //!< A name for debug purpose.

	unsigned long ref; //!< Allocated slice num.
	unsigned long max; //!< Max available. 0: unlimited
	unsigned long fail; //!< Save the malloc failure number for debug purpose.

	int (*ctor)(void *slice);
	void (*dtor)(void *slice);

	pthread_spinlock_t lock;

	struct list_head list_free; //!< Available memory slice. Store cache-maybe-hot at head.
};

#define DEFINE_MEMPOOL(_name) \
	static struct mempool _name = { .magic = 0 }

#define DEFINE_MEMPOOL_TLS(_name) \
	static __thread struct mempool _name = { .magic = 0 }

extern unsigned int mempool_calc_slice_size(const unsigned int input);

extern int mempool_init(
	struct mempool *mp,
	const char *name, const unsigned int size, const unsigned long max,
	int (*ctor)(void *), void (*dtor)(void *));
extern void mempool_exit(struct mempool *mp);

extern void *mempool_alloc(struct mempool *mp);
extern void mempool_free(struct mempool *mp, void *p);
extern void mempool_recycle(struct mempool *mp, const unsigned int reserve);

#endif /* SRC_MEMPOOL_MEMPOOL_H_ */
