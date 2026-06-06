/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Only give sleepers 50% of their service deficit. This allows
 * them to run sooner, but does not allow tons of sleepers to
 * rip the spread apart.
 */
SCHED_FEAT(GENTLE_FAIR_SLEEPERS, false)

/*
 * Place new tasks ahead so that they do not starve already running
 * tasks
 */
SCHED_FEAT(START_DEBIT, false)
SCHED_FEAT(PLACE_LAG, false)
SCHED_FEAT(PLACE_DEADLINE_INITIAL, false)
SCHED_FEAT(PLACE_REL_DEADLINE, false)
SCHED_FEAT(DELAY_DEQUEUE, false)
SCHED_FEAT(DELAY_ZERO, true)

/*
 * Prefer to schedule the task we woke last (assuming it failed
 * wakeup-preemption), since its likely going to consume data we
 * touched, increases cache locality.
 */
SCHED_FEAT(NEXT_BUDDY, false)

/*
 * Consider buddies to be cache hot, decreases the likeliness of a
 * cache buddy being migrated away, increases cache locality.
 */
SCHED_FEAT(CACHE_HOT_BUDDY, false)

/*
 * Allow wakeup-time preemption of the current task:
 */
SCHED_FEAT(WAKEUP_PREEMPTION, true)

SCHED_FEAT(HRTICK, false)
SCHED_FEAT(DOUBLE_TICK, false)
SCHED_FEAT(LB_BIAS, true)

/*
 * Decrement CPU capacity based on time not spent running tasks
 */
SCHED_FEAT(NONTASK_CAPACITY, true)

/*
 * Queue remote wakeups on the target CPU and process them
 * using the scheduler IPI. Reduces rq->lock contention/bounces.
 */
SCHED_FEAT(TTWU_QUEUE, false)

/*
 * When doing wakeups, attempt to limit superfluous scans of the LLC domain.
 */
SCHED_FEAT(SIS_AVG_CPU, false)

/*
 * Issue a WARN when we do multiple update_rq_clock() calls
 * in a single rq->lock section. Default disabled because the
 * annotations are not complete.
 */
SCHED_FEAT(WARN_DOUBLE_CLOCK, false)

#ifdef HAVE_RT_PUSH_IPI
/*
 * In order to avoid a thundering herd attack of CPUs that are
 * lowering their priorities at the same time, and there being
 * a single CPU that has an RT task that can migrate and is waiting
 * to run, where the other CPUs will try to take that CPUs
 * rq lock and possibly create a large contention, sending an
 * IPI to that CPU and let that CPU push the RT task to where
 * it should go may be a better scenario.
 *
 * This is best for PREEMPT_RT, but for non-RT it can cause issues
 * when preemption is disabled for long periods of time. Have
 * it only default enabled for PREEMPT_RT.
 */
# ifdef CONFIG_PREEMPT_RT
SCHED_FEAT(RT_PUSH_IPI, true)
# else
SCHED_FEAT(RT_PUSH_IPI, false)
# endif
#endif

SCHED_FEAT(RT_RUNTIME_SHARE, false)
SCHED_FEAT(LB_MIN, false)
SCHED_FEAT(ATTACH_AGE_LOAD, true)

SCHED_FEAT(WA_IDLE, true)
SCHED_FEAT(WA_WEIGHT, true)
SCHED_FEAT(WA_BIAS, true)

/*
 * UtilEstimation. Use estimated CPU utilization.
 */
SCHED_FEAT(UTIL_EST, true)
SCHED_FEAT(UTIL_EST_FASTUP, true)

/*
 * Energy aware scheduling. Use platform energy model to guide scheduling
 * decisions optimizing for energy efficiency.
 */
#ifdef CONFIG_DEFAULT_USE_ENERGY_AWARE
SCHED_FEAT(ENERGY_AWARE, true)
#else
SCHED_FEAT(ENERGY_AWARE, false)
#endif

/*
 * Energy aware scheduling algorithm choices:
 * EAS_PREFER_IDLE
 *   Direct tasks in a schedtune.prefer_idle=1 group through
 *   the EAS path for wakeup task placement. Otherwise, put
 *   those tasks through the mainline slow path.
 * FIND_BEST_TARGET
 *   Limit the number of placement options for which we calculate
 *   energy by using heuristics to select 'best idle' and
 *   'best active' cpu options.
 * FBT_STRICT_ORDER
 *   ON: If the target CPU saves any energy, use that.
 *   OFF: Use whichever of target or backup saves most.
 */
SCHED_FEAT(EAS_PREFER_IDLE, true)
SCHED_FEAT(FIND_BEST_TARGET, false)
SCHED_FEAT(FBT_STRICT_ORDER, false)

/*
 * Apply schedtune boost hold to tasks of all sched classes.
 * If enabled, schedtune will hold the boost applied to a CPU
 * for 50ms regardless of task activation - if the task is
 * still running 50ms later, the boost hold expires and schedtune
 * boost will expire immediately the task stops.
 * If disabled, this behaviour will only apply to tasks of the
 * RT class.
 */
SCHED_FEAT(SCHEDTUNE_BOOST_HOLD_ALL, false)

/*
 * Inflate the effective utilization of SchedTune-boosted tasks, which
 * generally leads to usage of higher frequencies.
 * If disabled, boosts will only bias tasks to higher-capacity CPUs.
 */
SCHED_FEAT(SCHEDTUNE_BOOST_UTIL, false)

SCHED_FEAT(EEVDF, true)

/*
 * EEVDF: Reject ineligible tasks during pick. When disabled, all runnable
 * tasks are treated as eligible (legacy CFS-like behavior).
 */
SCHED_FEAT(ENFORCE_ELIGIBILITY, false)

/*
 * EEVDF: Use overflow-checked sum_w_vruntime accumulation and scale down
 * weights via sum_shift when multiplication would overflow s64.
 */
SCHED_FEAT(PARANOID_AVG, false)

/*
 * EEVDF: Allow tasks with a shorter slice to wakeup-preempt the current task,
 * overriding slice protection. Improves latency for interactive tasks.
 */
SCHED_FEAT(PREEMPT_SHORT, true)

/*
 * EEVDF: Protect the current task's slice against preemption only up to the
 * minimum slice of all runnable tasks (run-to-parity). When disabled, the
 * full normalized base slice is used as the protection window.
 */
SCHED_FEAT(RUN_TO_PARITY, false)

/*
 * EEVDF: Honor the ->next buddy hint when picking the next task, allowing
 * cache-warm tasks to run ahead of strict deadline order.
 */
SCHED_FEAT(PICK_BUDDY, true)

/*
 * Do newidle balancing proportional to its success rate using randomization.
 */
SCHED_FEAT(NI_RANDOM, true)
SCHED_FEAT(NI_RATE, true)
