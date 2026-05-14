/*SPDX-License-Identifier: GPL-2.0
* CASS COMPATIBILITY HEADER
* by deu
* i hope these stubs will be completed one by one!
*/
#ifndef _CASS_4_14_COMPAT
#define _CASS_4_14_COMPAT

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include "pelt.h"

static inline unsigned long cpu_util_dl(struct rq *rq)
{
#ifdef CONFIG_SMP
    return READ_ONCE(rq->dl.avg.util_avg);
#else
    return 0;
#endif
}

static inline unsigned long thermal_load_avg(struct rq *rq)
{
    return 0;
}

#ifndef fits_capacity
inline bool fits_capacity(unsigned long util, unsigned long capacity)
{
    return util <= capacity;
}
#endif

#ifndef arch_scale_min_freq_capacity
static inline unsigned long arch_scale_min_freq_capacity(int cpu)
{
    return arch_scale_cpu_capacity(cpu);
}
#endif

static inline bool choose_idle_cpu(int cpu, struct task_struct *p)
{
    if (!idle_cpu(cpu))
        return false;
    if (vcpu_is_preempted(cpu))
        return false;
    return true;
}


#endif /* _CASS_4_14_COMPAT */
