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

#endif /* CONFIG_SMP */
