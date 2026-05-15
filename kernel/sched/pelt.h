#ifndef _KERNEL_SCHED_PELT_H
#define _KERNEL_SCHED_PELT_H

#ifdef CONFIG_SMP
#include "sched-pelt.h"

struct rt_rq;

/*
 * The UTIL_AVG_UNCHANGED flag is used to synchronize util_est with util_avg
 * updates. When a task is dequeued, its util_est should not be updated if its
 * util_avg has not been updated in the meantime.
 * This information is mapped into the MSB bit of util_est.enqueued at dequeue
 * time. Since max value of util_est.enqueued for a task is 1024 (PELT util_avg
 * for a task) it is safe to use MSB.
 */
#define UTIL_AVG_UNCHANGED	0x80000000
#define cap_scale(v, s) ((v)*(s) >> SCHED_CAPACITY_SHIFT)

int __update_load_avg_blocked_se(u64 now, int cpu, struct sched_entity *se);
int __update_load_avg_se(u64 now, int cpu, struct cfs_rq *cfs_rq, struct sched_entity *se);
int __update_load_avg_cfs_rq(u64 now, int cpu, struct cfs_rq *cfs_rq);
int update_rt_rq_load_avg(u64 now, int cpu, struct rt_rq *rt_rq, int running);
int update_dl_rq_load_avg(u64 now, int cpu, struct dl_rq *dl_rq, int running);
#ifdef CONFIG_IRQ_TIME_ACCOUNTING
int update_irq_load_avg(struct rq *rq, u64 running);

static inline unsigned long cpu_util_irq(struct rq *rq)
{
	return READ_ONCE(rq->avg_irq.util_avg);
}

static inline unsigned long scale_irq_capacity(unsigned long util,
						unsigned long irq,
						unsigned long max)
{
	util *= (max - irq);
	util /= max;
	return util;
}
#else
static inline unsigned long cpu_util_irq(struct rq *rq) { return 0; }
static inline unsigned long scale_irq_capacity(unsigned long util,
						unsigned long irq,
						unsigned long max)
{ return util; }
#endif

static inline u64 rq_clock_pelt(struct rq *rq)
{
	lockdep_assert_held(&rq->lock);
	assert_clock_updated(rq);

	return rq->clock_pelt - rq->lost_idle_time;
}

static inline void _update_idle_rq_clock_pelt(struct rq *rq)
{
	rq->clock_pelt = rq_clock_task(rq);
}

static inline void update_rq_clock_pelt(struct rq *rq, s64 delta)
{
	if (unlikely(is_idle_task(rq->curr))) {
		_update_idle_rq_clock_pelt(rq);
		return;
	}

	delta = cap_scale(delta, arch_scale_cpu_capacity(cpu_of(rq)));
	delta = cap_scale(delta, arch_scale_freq_capacity(cpu_of(rq)));

	rq->clock_pelt += delta;
}

static inline void update_idle_rq_clock_pelt(struct rq *rq)
{
	u32 divider = ((LOAD_AVG_MAX - 1024) << SCHED_CAPACITY_SHIFT) - LOAD_AVG_MAX;
	u32 util_sum = rq->cfs.avg.util_sum;
	util_sum += rq->rt.avg.util_sum;
	util_sum += rq->dl.avg.util_sum;

	if (util_sum >= divider)
		rq->lost_idle_time += rq_clock_task(rq) - rq->clock_pelt;

	_update_idle_rq_clock_pelt(rq);
}

#endif /* CONFIG_SMP */

#endif /* _KERNEL_SCHED_PELT_H */
