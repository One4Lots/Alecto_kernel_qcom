// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 deu <fawwazzuladhim700@gmail.com>.
 * schedutil_lut.h - Per-cluster util-to-frequency LUT for schedutil governor
 *
 * LUT keys are percentage of cluster capacity (0–100).
 * Lookup converts raw util to percentage via:
 *   util_pct = util * 100 / arch_scale_cpu_capacity(cpu)
 */

#ifndef _SUGOV_DVFS_LUT_H
#define _SUGOV_DVFS_LUT_H

struct sugov_lut_entry {
	unsigned int util_pct;   /* 0–100: percentage of cluster capacity */
	unsigned int freq_khz;
};

/* — Silver Cluster (cpu0-5) — Max: 1804800 kHz */
static const struct sugov_lut_entry sugov_lut_silver[] = {
	{   0,  576000 },
	{  34,  768000 },
	{  45, 1017600 },
	{  55, 1248000 },
	{  59, 1324800 },
	{  67, 1516800 },
	{  71, 1612800 },
	{  76, 1708800 },
	{ 100, 1804800 },
};

/* — Gold Cluster (cpu6-7) — Max: 2323200 kHz */
static const struct sugov_lut_entry sugov_lut_gold[] = {
	{   0,  652800 },
	{  28,  825600 },
	{  34,  979200 },
	{  38, 1113600 },
	{  44, 1267200 },
	{  54, 1555200 },
	{  59, 1708800 },
	{  63, 1843200 },
	{  65, 1900800 },
	{  69, 1996800 },
	{  73, 2112000 },
	{  76, 2208000 },
	{ 100, 2323200 },
};

#define SUGOV_LUT_SIZE(lut) (ARRAY_SIZE(lut))

/*
 * sugov_lut_lookup - interpolate freq from util percentage using a LUT
 * @lut:      pointer to the cluster's LUT array
 * @size:     number of entries (use SUGOV_LUT_SIZE)
 * @util_pct: util as percentage of cluster capacity (0–100)
 *
 * Returns frequency in kHz.
 */
static inline unsigned int sugov_lut_lookup(const struct sugov_lut_entry *lut,
					    unsigned int size,
					    unsigned int util_pct)
{
	unsigned int i;

	if (util_pct <= lut[0].util_pct)
		return lut[0].freq_khz;

	for (i = 1; i < size; i++) {
		if (util_pct <= lut[i].util_pct) {
			unsigned long du = lut[i].util_pct  - lut[i-1].util_pct;
			unsigned long df = lut[i].freq_khz  - lut[i-1].freq_khz;
			return lut[i-1].freq_khz +
			       (unsigned int)(df * (util_pct - lut[i-1].util_pct) / du);
		}
	}

	return lut[size - 1].freq_khz;
}

#endif /* _SUGOV_DVFS_LUT_H */
