// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 deu <fawwazzuladhim700@gmail.com>.
 * schedutil_lut.h - Per-cluster util-to-frequency LUT for schedutil governor
 */

#ifndef _SUGOV_DVFS_LUT_H
#define _SUGOV_DVFS_LUT_H

struct sugov_lut_entry {
	unsigned int hroom_util;
	unsigned int freq_khz;
};

/* — Silver Cluster (cpu0-5) — Max: 1804800 kHz */
static const struct sugov_lut_entry sugov_lut_silver[] = {
	{    0,  576000 },
	{  349,  768000 },
	{  462, 1017600 },
	{  566, 1248000 },
	{  601, 1324800 },
	{  688, 1516800 },
	{  732, 1612800 },
	{  776, 1708800 },
	{ 1024, 1804800 },
};

/* — Gold Cluster (cpu6-7) — Max: 2323200 kHz */
static const struct sugov_lut_entry sugov_lut_gold[] = {
	{    0,  652800 },
	{  291,  825600 },
	{  345,  979200 },
	{  393, 1113600 },
	{  447, 1267200 },
	{  548, 1555200 },
	{  603, 1708800 },
	{  650, 1843200 },
	{  670, 1900800 },
	{  704, 1996800 },
	{  745, 2112000 },
	{  779, 2208000 },
	{ 1024, 2323200 },
};

#define SUGOV_LUT_SIZE(lut) (ARRAY_SIZE(lut))

/*
 * sugov_lut_lookup - interpolate freq from hroom util using a LUT
 * @lut:  pointer to the cluster's LUT array
 * @size: number of entries (use SUGOV_LUT_SIZE)
 * @util:  util value (0-1024)
 *
 * Returns frequency in kHz.
 */
static inline unsigned int sugov_lut_lookup(const struct sugov_lut_entry *lut,
					    unsigned int size,
					    unsigned long util)
{
	unsigned int i;

	if (util <= lut[0].hroom_util)
		return lut[0].freq_khz;

	for (i = 1; i < size; i++) {
		if (util <= lut[i].hroom_util) {
			unsigned long du = lut[i].hroom_util - lut[i-1].hroom_util;
			unsigned long df = lut[i].freq_khz   - lut[i-1].freq_khz;
			return lut[i-1].freq_khz +
			       (unsigned int)(df * (util - lut[i-1].hroom_util) / du);
		}
	}

	return lut[size - 1].freq_khz;
}

#endif /* _SUGOV_DVFS_LUT_H */
