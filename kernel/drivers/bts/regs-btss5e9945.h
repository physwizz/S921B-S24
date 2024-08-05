/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License Version 2 as publised
 * by the Free Software Foundation.
 *
 * Header for BTS Bus Traffic Shaper
 *
 * Includes Register information to control BTS devices
 *
 */

#define CON		0x000
#define TIMEOUT		0x010
#define RCON		0x020
#define RBLK_UPPER	0x024
#define RBLK_UPPER_FULL	0x02C
#define RBLK_UPPER_BUSY	0x030
#define RBLK_UPPER_MAX	0x034
#define WCON		0x040
#define WBLK_UPPER	0x044
#define WBLK_UPPER_FULL	0x04C
#define WBLK_UPPER_BUSY	0x050
#define WBLK_UPPER_MAX	0x054
#define CORE_QOS_EN	0x4
#define TIMEOUT_R0	0x008
#define TIMEOUT_R1	0x00C
#define TIMEOUT_W0	0x010
#define TIMEOUT_W1	0x014

#define REGOFF_QU_TIMEOUT_CNT			0x10
#define	SHIFT_QU_TIMEOUT_CNT_INITIAL_R	0
#define MASK_QU_TIMEOUT_CNT_INITIAL_R	0xff
#define	SHIFT_QU_TIMEOUT_CNT_INITIAL_W	16
#define MASK_QU_TIMEOUT_CNT_INITIAL_W	0xff

#define AXQOS_BYPASS	8
#define AXQOS_VAL	12

#define SCIQOS_EN	0
#define SCIQOS_R	2
#define SCIQOS_W	0

#define AXQOS_ONOFF	0
#define BLOCK_UPPER	0
#define BLOCK_UPPER0	0
#define BLOCK_UPPER1	16
#define TIMEOUT_CNT_R	0
#define TIMEOUT_CNT_W	16
#define QURGENT_EN	20
#define QURGENT_EX	4
#define BLOCKING_EN	0
#define TIMEOUT_CNT_VC	0

#define RMO_PORT_0	0
#define RMO_PORT_1	16
#define WMO_PORT_0	8
#define WMO_PORT_1	24

#define SCI_CTRL	0x0000
#define CRP_CTL3_0	0x10
#define CRP_CTL3_1	0x38
#define CRP_CTL3_2	0x60
#define CRP_CTL3_3	0x88
#define TH_IMM_R_0	0x010
#define TH_IMM_W_0	0x018
#define TH_HIGH_R_0	0x020
#define TH_HIGH_W_0	0x028
#define TH_IMM_R_VAL 0x10101010
#define TH_IMM_W_VAL 0x28282828
#define TH_HIGH_R_VAL 0x10101010
#define TH_HIGH_W_VAL 0x28282828

#define IOREMAP_OFFSET		(0x100)
#define CRP0_P0_CTRL	(0x188 - IOREMAP_OFFSET)
#define CRP1_P0_CTRL	(0x1A8 - IOREMAP_OFFSET)
#define CRP2_P0_CTRL	(0x1C8 - IOREMAP_OFFSET)
#define CRP3_P0_CTRL	(0x1E8 - IOREMAP_OFFSET)

#define CRP0_P1_CTRL	(0x18C - IOREMAP_OFFSET)
#define CRP1_P1_CTRL	(0x1AC - IOREMAP_OFFSET)
#define CRP2_P1_CTRL	(0x1CC - IOREMAP_OFFSET)
#define CRP3_P1_CTRL	(0x1EC - IOREMAP_OFFSET)
#define HURRYLEVEL3MO_SHIFT 16

#define HIGH_THRESHOLD_SHIFT	24
#define MID_THRESHOLD_SHIFT	16
#define HIGH_THRESOLD	0xB
#define MID_THRESHOLD	0x7

#define DEFAULT_QBUSY_TH	0x4

#define SMC_SCHEDCTL_BUNDLE_CTRL4	0x0

#define QMAX_THRESHOLD_SHIFT 16
#define DEFAULT_QMAX_RD_TH	0x60
#define DEFAULT_QMAX_WR_TH	0x30

#define QMAX_THRESHOLD_R	0x0
#define QMAX_THRESHOLD_W	0x4

#define MIN_BW			4715211
#define MID_BW			9464012
#define MAX_BW			11356812

#define SUB_RATIO		1382880
#define READ5L			7136610
#define READHD			5074863
