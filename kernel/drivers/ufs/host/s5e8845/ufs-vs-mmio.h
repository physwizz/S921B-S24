// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2013-2016, Linux Foundation. All rights reserved.
 */

#ifndef _UFS_VS_HANDLE_H_
#define _UFS_VS_HANDLE_H_

struct ufs_vs_handle {
	void *std;	/* Need to care conditions */
	void *ufsp;
	void *unipro;
	void *pma;
	void *cport;
	void *pcs;
	void *mcq;
	void *sqcq;
	void (*udelay)(u32 us);
	void *private;
};

#define EXYNOS_UFS_MMIO_FUNC(name)					\
static inline void name##_writel(struct ufs_vs_handle *handle, u32 val, u32 ofs)	\
{									\
	writel(val, handle->name + ofs);				\
}									\
static inline u32 name##_readl(struct ufs_vs_handle *handle, u32 ofs)	\
{									\
	return readl(handle->name + ofs);				\
}

EXYNOS_UFS_MMIO_FUNC(std);
EXYNOS_UFS_MMIO_FUNC(ufsp);
EXYNOS_UFS_MMIO_FUNC(unipro);
EXYNOS_UFS_MMIO_FUNC(cport);
EXYNOS_UFS_MMIO_FUNC(pcs);
EXYNOS_UFS_MMIO_FUNC(mcq);
EXYNOS_UFS_MMIO_FUNC(sqcq);

#define hci_writel(handle, val, ofs)    std_writel(handle, val, ofs)
#define hci_readl(handle, ofs)          std_readl(handle, ofs)

#if defined(__UFS_CAL_FW__)

#undef MISC_CAL
#undef MPHY_APBCLK_CAL

#define	MISC_CAL		0x11B4
#define	MPHY_APBCLK_CAL	(1 << 10)

static inline void pma_writel(struct ufs_vs_handle *handle, u32 val, u32 ofs)
{
	u32 clkstop_ctrl = readl(handle->std + MISC_CAL);

	writel(clkstop_ctrl & ~MPHY_APBCLK_CAL, handle->std + MISC_CAL);
	writel(val, handle->pma + ofs);
	writel(clkstop_ctrl | MPHY_APBCLK_CAL, handle->std + MISC_CAL);
}

static inline u32 pma_readl(struct ufs_vs_handle *handle, u32 ofs)
{
	u32 clkstop_ctrl = readl(handle->std + MISC_CAL);
	u32 val;

	writel(clkstop_ctrl & ~MPHY_APBCLK_CAL, handle->std + MISC_CAL);
	val = readl(handle->pma + ofs);
	writel(clkstop_ctrl | MPHY_APBCLK_CAL, handle->std + MISC_CAL);
	return val;
}
#else
EXYNOS_UFS_MMIO_FUNC(pma);
#endif

#endif /* _UFS_VS_HANDLE_H_ */