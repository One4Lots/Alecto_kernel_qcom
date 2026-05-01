#ifdef CONFIG_SMP
#include "sched-pelt.h"

#define UTIL_AVG_UNCHANGED 0x1
#define cap_scale(v, s) ((v)*(s) >> SCHED_CAPACITY_SHIFT)

int __update_load_avg_blocked_se(u64 now, int cpu, struct sched_entity *se);
int __update_load_avg_se(u64 now, int cpu, struct cfs_rq *cfs_rq, struct sched_entity *se);
int __update_load_avg_cfs_rq(u64 now, int cpu, struct cfs_rq *cfs_rq);
int update_rt_rq_load_avg(u64 now, int cpu, struct rt_rq *rt_rq, int running);

#endif /* CONFIG_SMP */
