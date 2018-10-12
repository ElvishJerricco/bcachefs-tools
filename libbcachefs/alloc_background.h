#ifndef _BCACHEFS_ALLOC_BACKGROUND_H
#define _BCACHEFS_ALLOC_BACKGROUND_H

#include "bcachefs.h"
#include "alloc_types.h"
#include "debug.h"

#define ALLOC_SCAN_BATCH(ca)		((ca)->mi.nbuckets >> 9)

const char *bch2_alloc_invalid(const struct bch_fs *, struct bkey_s_c);
int bch2_alloc_to_text(struct bch_fs *, char *, size_t, struct bkey_s_c);

#define bch2_bkey_alloc_ops (struct bkey_ops) {		\
	.key_invalid	= bch2_alloc_invalid,		\
	.val_to_text	= bch2_alloc_to_text,		\
}

int bch2_alloc_read(struct bch_fs *, struct list_head *);
int bch2_alloc_replay_key(struct bch_fs *, struct bpos);

static inline void bch2_wake_allocator(struct bch_dev *ca)
{
	struct task_struct *p;

	rcu_read_lock();
	p = rcu_dereference(ca->alloc_thread);
	if (p)
		wake_up_process(p);
	rcu_read_unlock();
}

static inline void verify_not_on_freelist(struct bch_fs *c, struct bch_dev *ca,
					  size_t bucket)
{
	if (expensive_debug_checks(c) &&
	    test_bit(BCH_FS_ALLOCATOR_STARTED, &c->flags)) {
		size_t iter;
		long i;
		unsigned j;

		for (j = 0; j < RESERVE_NR; j++)
			fifo_for_each_entry(i, &ca->free[j], iter)
				BUG_ON(i == bucket);
		fifo_for_each_entry(i, &ca->free_inc, iter)
			BUG_ON(i == bucket);
	}
}

void bch2_recalc_capacity(struct bch_fs *);

void bch2_dev_allocator_remove(struct bch_fs *, struct bch_dev *);
void bch2_dev_allocator_add(struct bch_fs *, struct bch_dev *);

void bch2_dev_allocator_stop(struct bch_dev *);
int bch2_dev_allocator_start(struct bch_dev *);

int bch2_alloc_write(struct bch_fs *);
int bch2_fs_allocator_start(struct bch_fs *);
void bch2_fs_allocator_init(struct bch_fs *);

#endif /* _BCACHEFS_ALLOC_BACKGROUND_H */
