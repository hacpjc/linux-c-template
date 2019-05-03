#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "lgu/lgu.h"
#include "mempool.h"

struct mempool_slice
{
#define MEMPOOL_SLICE_MAGIC (54085408)
	unsigned int magic;
	struct list_head list;
	uint8_t buf[0];
};

static struct mempool_slice *alloc_slice(struct mempool *mp)
{
	struct mempool_slice *slice;

	if (list_empty(&mp->list_free))
	{
		/*
		 * No more slice available. Allocate more, but do not exceed limit.
		 */
		if (mp->max && mp->ref >= mp->max)
		{
			mp->fail++;
			return NULL;
		}

		BUG_ON(mp->sz < sizeof(struct mempool_slice));
		slice = malloc(mp->sz);
		if (!slice)
		{
			return NULL;
		}

		mp->ref++;
	}
	else
	{
		/*
		 * Get from hot cache. (possibly hot)
		 */
		slice = list_first_entry(&mp->list_free, struct mempool_slice, list);
		list_del(&slice->list);

		BUG_ON(slice->magic != MEMPOOL_SLICE_MAGIC);
	}

	return slice;
}

static void free_slice(struct mempool *mp, struct mempool_slice *slice)
{
	BUG_ON(slice->magic != MEMPOOL_SLICE_MAGIC);
	free(slice);

	mp->ref--;
}

void *mempool_alloc(struct mempool *mp)
{
	struct mempool_slice *slice;

	pthread_spin_lock(&mp->lock);
	{
		slice = alloc_slice(mp);
	}
	pthread_spin_unlock(&mp->lock);

	if (mp->ctor)
	{
		if (mp->ctor((void *) slice))
		{
			/*
			 * Caller reject this allocation.
			 */
			mempool_free(mp, slice);
			return NULL;
		}
	}

	return slice;
}

void mempool_free(struct mempool *mp, void *p)
{
	struct mempool_slice *slice = (struct mempool_slice *) p;

	if (mp->dtor)
	{
		mp->dtor(p);
	}

	slice->magic = MEMPOOL_SLICE_MAGIC;

	pthread_spin_lock(&mp->lock);
	list_add(&slice->list, &mp->list_free);
	pthread_spin_unlock(&mp->lock);
}

static inline void __mempool_recycle(struct mempool *mp, const unsigned int reserve)
{
	if (!list_empty(&mp->list_free))
	{
		struct mempool_slice *slice, *slice_save;
		list_for_each_entry_safe(slice, slice_save, &mp->list_free, list)
		{
			if (mp->ref <= reserve)
			{
				break;
			}

			list_del(&slice->list);
			free_slice(mp, slice);
		}
	}
}

void mempool_recycle(struct mempool *mp, const unsigned int reserve)
{
	pthread_spin_lock(&mp->lock);
	__mempool_recycle(mp, 0);
	pthread_spin_unlock(&mp->lock);
}

/*!
 * @brief Calculate a proper size for one slice.
 */
unsigned int mempool_calc_slice_size(const unsigned int input)
{
	unsigned int output, real_size = input;
	const unsigned int minimal_size = sizeof(struct mempool_slice);

	if (real_size < minimal_size)
	{
		real_size = minimal_size;
	}

	{
#define ALIGN_TO_N (sizeof(unsigned long))
		unsigned int padding;

		if (real_size % ALIGN_TO_N)
		{
			padding = ALIGN_TO_N - (real_size % ALIGN_TO_N);
		}
		else
		{
			padding = 0;
		}

		output = real_size + padding;
	}

	return output;
}

int mempool_init(
	struct mempool *mp,
	const char *name, const unsigned int size, const unsigned long max,
	int (*ctor)(void *), void (*dtor)(void *))
{
	BUG_ON(mp == NULL);
	BUG_ON(name == NULL || strlen(name) == 0);

	snprintf(mp->name, sizeof(mp->name), "%s", name);

	mp->magic = 0x54085408;
	mp->sz = mempool_calc_slice_size(size);
	mp->max = max; // 0: no limit.
	mp->ref = 0;
	mp->ctor = ctor;
	mp->dtor = dtor;

	mp->fail = 0;

	INIT_LIST_HEAD(&mp->list_free);

	pthread_spin_init(&mp->lock, 0);

	return 0;
}

void mempool_exit(struct mempool *mp)
{
	/*
	 * Recycle
	 */
	__mempool_recycle(mp, 0);

	/*
	 * Detect memory leakage.
	 */
	if (mp->ref != 0)
	{
		fprintf(stderr, " * ERROR: Detect %lu dirty leakage at %s\n", (unsigned long) mp->ref, mp->name);
	}

	pthread_spin_destroy(&mp->lock);
}
