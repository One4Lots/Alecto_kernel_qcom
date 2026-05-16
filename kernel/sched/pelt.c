// SPDX-License-Identifier: GPL-2.0
/*
 * Per Entity Load Tracking
 *
 *  Copyright (C) 2007 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *
 *  Interactivity improvements by Mike Galbraith
 *  (C) 2007 Mike Galbraith <efault@gmx.de>
 *
 *  Various enhancements by Dmitry Adamushko.
 *  (C) 2007 Dmitry Adamushko <dmitry.adamushko@gmail.com>
 *
 *  Group scheduling enhancements by Srivatsa Vaddagiri
 *  Copyright IBM Corporation, 2007
 *  Author: Srivatsa Vaddagiri <vatsa@linux.vnet.ibm.com>
 *
 *  Scaled math optimizations by Thomas Gleixner
 *  Copyright (C) 2007, Thomas Gleixner <tglx@linutronix.de>
 *
 *  Adaptive scheduling granularity, math enhancements by Peter Zijlstra
 *  Copyright (C) 2007 Red Hat, Inc., Peter Zijlstra
 *
 *  Move PELT related code from fair.c into this pelt.c file
 *  Author: Vincent Guittot <vincent.guittot@linaro.org>
 */

#include <linux/sched.h>
#include "sched.h"
#include "pelt.h"

#include <trace/events/sched.h>
/*
 * Approximate:
 *   val * y^n,    where y^32 ~= 0.5 (~1 scheduling period)
 */
static u64 decay_load(u64 val, u64 n)
{
	unsigned int local_n;

	if (unlikely(n > LOAD_AVG_PERIOD * 63))
		return 0;

	/* after bounds checking we can collapse to 32-bit */
	local_n = n;

	/*
	 * As y^PERIOD = 1/2, we can combine
	 *    y^n = 1/2^(n/PERIOD) * y^(n%PERIOD)
	 * With a look-up table which covers y^n (n<PERIOD)
	 *
	 * To achieve constant time decay_load.
	 */
	if (unlikely(local_n >= LOAD_AVG_PERIOD)) {
		val >>= local_n / LOAD_AVG_PERIOD;
		local_n %= LOAD_AVG_PERIOD;
	}

	val = mul_u64_u32_shr(val, runnable_avg_yN_inv[local_n], 32);
	return val;
}

static u32 __accumulate_pelt_segments(u64 periods, u32 d1, u32 d3)
{
	u32 c1, c2, c3 = d3; /* y^0 == 1 */

	/*
	 * c1 = d1 y^p
	 */
	c1 = decay_load((u64)d1, periods);

	/*
	 *            p-1
	 * c2 = 1024 \Sum y^n
	 *            n=1
	 *
	 *              inf        inf
	 *    = 1024 ( \Sum y^n - \Sum y^n - y^0 )
	 *              n=0        n=p
	 */
	c2 = LOAD_AVG_MAX - decay_load(LOAD_AVG_MAX, periods) - 1024;

	return c1 + c2 + c3;
}

/*
 * Accumulate the three separate parts of the sum; d1 the remainder
 * of the last (incomplete) period, d2 the span of full periods and d3
 * the remainder of the (incomplete) current period.
 *
 *           d1          d2           d3
 *           ^           ^            ^
 *           |           |            |
 *         |<->|<----------------->|<--->|
 * ... |---x---|------| ... |------|-----x (now)
 *
 *                           p-1
 * u' = (u + d1) y^p + 1024 \Sum y^n + d3 y^0
 *                           n=1
 *
 *    = u y^p +					(Step 1)
 *
 *                     p-1
 *      d1 y^p + 1024 \Sum y^n + d3 y^0		(Step 2)
 *                     n=1
 */
static __always_inline u32
accumulate_sum(u64 delta, int cpu, struct sched_avg *sa,
	       unsigned long weight, int running, struct cfs_rq *cfs_rq)
{
	unsigned long scale_freq, scale_cpu;
	u32 contrib = (u32)delta; /* p == 0 -> delta < 1024 */
	u64 periods;

	scale_freq = arch_scale_freq_capacity(cpu);
	scale_cpu = arch_scale_cpu_capacity(cpu);

	delta += sa->period_contrib;
	periods = delta / 1024; /* A period is 1024us (~1ms) */

	/*
	 * Step 1: decay old *_sum if we crossed period boundaries.
	 */
	if (periods) {
		sa->load_sum = decay_load(sa->load_sum, periods);
		if (cfs_rq) {
			cfs_rq->runnable_load_sum =
				decay_load(cfs_rq->runnable_load_sum, periods);
		}
		sa->util_sum = decay_load((u64)(sa->util_sum), periods);

		/*
		 * Step 2
		 */
		delta %= 1024;
		contrib = __accumulate_pelt_segments(periods,
				1024 - sa->period_contrib, delta);
	}
	sa->period_contrib = delta;

	contrib = cap_scale(contrib, scale_freq);
	if (weight) {
		sa->load_sum += weight * contrib;
		if (cfs_rq)
			cfs_rq->runnable_load_sum += weight * contrib;
	}
	if (running)
		sa->util_sum += contrib * scale_cpu;

	return periods;
}

/*
 * We can represent the historical contribution to runnable average as the
 * coefficients of a geometric series.  To do this we sub-divide our runnable
 * history into segments of approximately 1ms (1024us); label the segment that
 * occurred N-ms ago p_N, with p_0 corresponding to the current period, e.g.
 *
 * [<- 1024us ->|<- 1024us ->|<- 1024us ->| ...
 *      p0            p1           p2
 *     (now)       (~1ms ago)  (~2ms ago)
 *
 * Let u_i denote the fraction of p_i that the entity was runnable.
 *
 * We then designate the fractions u_i as our co-efficients, yielding the
 * following representation of historical load:
 *   u_0 + u_1*y + u_2*y^2 + u_3*y^3 + ...
 *
 * We choose y based on the with of a reasonably scheduling period, fixing:
 *   y^32 = 0.5
 *
 * This means that the contribution to load ~32ms ago (u_32) will be weighted
 * approximately half as much as the contribution to load within the last ms
 * (u_0).
 *
 * When a period "rolls over" and we have new u_0`, multiplying the previous
 * sum again by y is sufficient to update:
 *   load_avg = u_0` + y*(u_0 + u_1*y + u_2*y^2 + ... )
 *            = u_0 + u_1*y + u_2*y^2 + ... [re-labeling u_i --> u_{i+1}]
 */
static __always_inline int
___update_load_avg(u64 now, int cpu, struct sched_avg *sa,
		  unsigned long weight, int running, struct cfs_rq *cfs_rq,
		  struct rt_rq *rt_rq)
{
	u64 delta;

	delta = now - sa->last_update_time;
	/*
	 * This should only happen when time goes backwards, which it
	 * unfortunately does during sched clock init when we swap over to TSC.
	 */
	if ((s64)delta < 0) {
		sa->last_update_time = now;
		return 0;
	}

	/*
	 * Use 1024ns as the unit of measurement since it's a reasonable
	 * approximation of 1us and fast to compute.
	 */
	delta >>= 10;
	if (!delta)
		return 0;

	sa->last_update_time += delta << 10;

	/*
	 * running is a subset of runnable (weight) so running can't be set if
	 * runnable is clear. But there are some corner cases where the current
	 * se has been already dequeued but cfs_rq->curr still points to it.
	 * This means that weight will be 0 but not running for a sched_entity
	 * but also for a cfs_rq if the latter becomes idle. As an example,
	 * this happens during newidle_balance() which calls
	 * update_blocked_averages()
	 */
	if (!weight)
		running = 0;

	/*
	 * Now we know we crossed measurement unit boundaries. The *_avg
	 * accrues by two steps:
	 *
	 * Step 1: accumulate *_sum since last_update_time. If we haven't
	 * crossed period boundaries, finish.
	 */
	if (!accumulate_sum(delta, cpu, sa, weight, running, cfs_rq))
		return 0;

	/*
	 * Step 2: update *_avg.
	 */
	sa->load_avg = div_u64(sa->load_sum, LOAD_AVG_MAX - 1024 + sa->period_contrib);
	if (cfs_rq) {
		cfs_rq->runnable_load_avg =
			div_u64(cfs_rq->runnable_load_sum, LOAD_AVG_MAX - 1024 + sa->period_contrib);
	}
	sa->util_avg = sa->util_sum / (LOAD_AVG_MAX - 1024 + sa->period_contrib);

	if (cfs_rq)
		trace_sched_load_cfs_rq(cfs_rq);
	else {
		if (likely(!rt_rq))
			trace_sched_load_se(container_of(sa, struct sched_entity, avg));
		else
			trace_sched_load_rt_rq(cpu, rt_rq);
	}

	return 1;
}

static inline void cfs_se_util_change(struct sched_avg *avg)
{
	unsigned int enqueued;

	if (!sched_feat(UTIL_EST))
		return;

	/* Avoid store if the flag has been already set */
	enqueued = avg->util_est.enqueued;
	if (!(enqueued & UTIL_AVG_UNCHANGED))
		return;

	/* Reset flag to report util_avg has been updated */
	enqueued &= ~UTIL_AVG_UNCHANGED;
	WRITE_ONCE(avg->util_est.enqueued, enqueued);
}

int
__update_load_avg_blocked_se(u64 now, int cpu, struct sched_entity *se)
{
	return ___update_load_avg(now, cpu, &se->avg, 0, 0, NULL, NULL);
}

int
__update_load_avg_se(u64 now, int cpu, struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	if (___update_load_avg(now, cpu, &se->avg,
			       se->on_rq * scale_load_down(se->load.weight),
			       cfs_rq->curr == se, NULL, NULL)) {
		cfs_se_util_change(&se->avg);

#ifdef UTIL_EST_DEBUG
		/*
		 * Trace utilization only for actual tasks.
		 *
		 * These trace events are mostly useful to get easier to
		 * read plots for the estimated utilization, where we can
		 * compare it with the actual grow/decrease of the original
		 * PELT signal.
		 * Let's keep them disabled by default in "production kernels".
		 */
		if (entity_is_task(se)) {
			struct task_struct *tsk = task_of(se);

			trace_sched_util_est_task(tsk, &se->avg);

			/* Trace utilization only for top level CFS RQ */
			cfs_rq = &(task_rq(tsk)->cfs);
			trace_sched_util_est_cpu(cpu, cfs_rq);
		}
#endif /* UTIL_EST_DEBUG */

		return 1;
	}

	return 0;
}

int
__update_load_avg_cfs_rq(u64 now, int cpu, struct cfs_rq *cfs_rq)
{
	return ___update_load_avg(now, cpu, &cfs_rq->avg,
			scale_load_down(cfs_rq->load.weight),
			cfs_rq->curr != NULL, cfs_rq, NULL);
}

int update_rt_rq_load_avg(u64 now, int cpu, struct rt_rq *rt_rq, int running)
{
	return ___update_load_avg(now, cpu, &rt_rq->avg, running, running, NULL, rt_rq);
}

/*
 * dl_rq:
 *
 *   util_sum = cpu_scale * load_sum
 *   runnable_sum = util_sum
 *
 *   load_avg and runnable_avg are not supported and meaningless.
 */
int update_dl_rq_load_avg(u64 now, int cpu, struct dl_rq *dl_rq, int running)
{
	return ___update_load_avg(now, cpu, &dl_rq->avg, running, running, NULL, NULL);
}