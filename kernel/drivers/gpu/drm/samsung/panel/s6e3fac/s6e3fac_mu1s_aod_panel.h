/*
 * linux/drivers/video/fbdev/exynos/panel/s6e3fac/s6e3fac_mu1s_aod_panel.h
 *
 * Header file for AOD Driver
 *
 * Copyright (c) 2016 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __S6E3FAC_MU1S_AOD_PANEL_H__
#define __S6E3FAC_MU1S_AOD_PANEL_H__

#include "oled_common_aod.h"
#include "oled_function.h"
#include "s6e3fac_mu1s_self_mask_img.h"

static u8 S6E3FAC_MU1S_AOD_KEY1_ENABLE[] = { 0xF0, 0x5A, 0x5A };
static u8 S6E3FAC_MU1S_AOD_KEY1_DISABLE[] = { 0xF0, 0xA5, 0xA5 };

static u8 S6E3FAC_MU1S_AOD_KEY2_ENABLE[] = { 0xF1, 0x5A, 0x5A };
static u8 S6E3FAC_MU1S_AOD_KEY2_DISABLE[] = { 0xF1, 0xA5, 0xA5 };

static u8 S6E3FAC_MU1S_AOD_KEY3_ENABLE[] = { 0xFC, 0x5A, 0x5A };
static u8 S6E3FAC_MU1S_AOD_KEY3_DISABLE[] = { 0xFC, 0xA5, 0xA5 };

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l1_key_enable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_KEY1_ENABLE, 0);
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l1_key_disable, DSI_PKT_TYPE_WR,S6E3FAC_MU1S_AOD_KEY1_DISABLE, 0);

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l2_key_enable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_KEY2_ENABLE, 0);
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l2_key_disable, DSI_PKT_TYPE_WR,S6E3FAC_MU1S_AOD_KEY2_DISABLE, 0);

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l3_key_enable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_KEY3_ENABLE, 0);
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_l3_key_disable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_KEY3_DISABLE, 0);

static DEFINE_PANEL_MDELAY(s6e3fac_mu1s_aod_self_spsram_write_delay, 1);
static DEFINE_PANEL_UDELAY(s6e3fac_mu1s_aod_self_spsram_sel_delay, 1);

static DEFINE_PANEL_UDELAY(s6e3fac_mu1s_aod_self_mask_checksum_1frame_delay, 16700);
static DEFINE_PANEL_UDELAY(s6e3fac_mu1s_aod_self_mask_checksum_2frame_delay, 33400);
static DEFINE_PANEL_UDELAY(s6e3fac_mu1s_aod_time_update_delay, 33400);


static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l1_key_enable, CMD_LEVEL_1,
	KEY_ENABLE, &PKTINFO(s6e3fac_mu1s_aod_l1_key_enable));
static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l1_key_disable, CMD_LEVEL_1,
	KEY_DISABLE, &PKTINFO(s6e3fac_mu1s_aod_l1_key_disable));

static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l2_key_enable, CMD_LEVEL_2,
	KEY_ENABLE, &PKTINFO(s6e3fac_mu1s_aod_l2_key_enable));
static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l2_key_disable, CMD_LEVEL_2,
	KEY_DISABLE, &PKTINFO(s6e3fac_mu1s_aod_l2_key_disable));

static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l3_key_enable, CMD_LEVEL_3,
	KEY_ENABLE, &PKTINFO(s6e3fac_mu1s_aod_l3_key_enable));
static DEFINE_PANEL_KEY(s6e3fac_mu1s_aod_l3_key_disable, CMD_LEVEL_3,
	KEY_DISABLE, &PKTINFO(s6e3fac_mu1s_aod_l3_key_disable));

static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l1_key_enable);
static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l1_key_disable);
static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l2_key_enable);
static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l2_key_disable);
static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l3_key_enable);
static struct keyinfo KEYINFO(s6e3fac_mu1s_aod_l3_key_disable);

static struct maptbl s6e3fac_mu1s_aod_maptbl[] = {
	[SET_PARTIAL_MODE_MAPTBL] = __OLED_MAPTBL_COPY_ONLY_INITIALIZER(s6e3fac_mu1s_aod_partial_mode, &OLED_FUNC(OLED_MAPTBL_COPY_AOD_PARTIAL_MODE)),
	[SET_PARTIAL_AREA_MAPTBL] = __OLED_MAPTBL_COPY_ONLY_INITIALIZER(s6e3fac_mu1s_aod_partial_area, &OLED_FUNC(OLED_MAPTBL_COPY_AOD_PARTIAL_AREA)),
	[SET_PARTIAL_HLPM_MAPTBL] = __OLED_MAPTBL_COPY_ONLY_INITIALIZER(s6e3fac_mu1s_aod_partial_hlpm, &OLED_FUNC(OLED_MAPTBL_COPY_AOD_PARTIAL_HLPM)),
};

// --------------------- Image for self mask image ---------------------

static char S6E3FAC_MU1S_AOD_RESET_SD_PATH[] = {
	0x75,
	0x00,
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_reset_sd_path, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_RESET_SD_PATH, 0);


static char S6E3FAC_MU1S_AOD_SELF_MASK_SD_PATH[] = {
	0x75,
	0x10,
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_sd_path, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_SD_PATH, 0);

// --------------------- End of self mask image ---------------------


// --------------------- Image for self mask control ---------------------
static char S6E3FAC_MU1S_AOD_FACTORY_SELF_MASK_ENA[] = {
	0x7A,
	0x05, 0x00, 0x00, 0x00, 0x09, 0x24, 0x09, 0x25,
	0x09, 0x26, 0x09, 0x27,
};

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_factory_self_mask_ctrl_ena,
		DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_FACTORY_SELF_MASK_ENA, 0);

static char S6E3FAC_MU1S_AOD_SELF_MASK_ENA[] = {
	0x7A,
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2B,
	0x07, 0xF8, 0x09, 0x23,
};

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_ctrl_ena,
		DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_ENA, 0);

static DEFINE_RULE_BASED_COND(mu1s_cond_is_factory_selfmask,
		PANEL_PROPERTY_IS_FACTORY_MODE, EQ, 1);

static void *s6e3fac_mu1s_aod_self_mask_ena_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&CONDINFO_IF(mu1s_cond_is_factory_selfmask),
		&PKTINFO(s6e3fac_mu1s_aod_factory_self_mask_ctrl_ena),
	&CONDINFO_EL(mu1s_cond_is_factory_selfmask),
		&PKTINFO(s6e3fac_mu1s_aod_self_mask_ctrl_ena),
	&CONDINFO_FI(mu1s_cond_is_factory_selfmask),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

static char S6E3FAC_MU1S_AOD_SELF_MASK_DISABLE[] = {
	0x7A,
	0x00,
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_disable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_DISABLE, 0);

static void *s6e3fac_mu1s_aod_self_mask_dis_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_disable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};
// --------------------- End of self mask control ---------------------

// --------------------- check sum control ----------------------------
static DEFINE_STATIC_PACKET_WITH_OPTION(s6e3fac_mu1s_aod_self_mask_crc_img_pkt,
		DSI_PKT_TYPE_WR_SR, S6E3FAC_MU1S_CRC_SELF_MASK_IMG, 0, PKT_OPTION_SR_ALIGN_16);

static char S6E3FAC_MU1S_AOD_SELF_MASK_CRC_ON1[] = {
	0xD8,
	0x17,
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_crc_on1, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_CRC_ON1, 0x27);

static char S6E3FAC_MU1S_AOD_SELF_MASK_DBIST_ON[] = {
	0xBF,
	0x01, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_dbist_on, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_DBIST_ON, 0);

static char S6E3FAC_MU1S_AOD_SELF_MASK_DBIST_OFF[] = {
	0xBF,
	0x00, 0x07, 0xFF, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_dbist_off, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_DBIST_OFF, 0);

static char S6E3FAC_MU1S_AOD_SELF_MASK_ENABLE_FOR_CHECKSUM[] = {
	0x7A,
	0x05, 0x00, 0x00, 0x00, 0x01, 0xF4, 0x02, 0x33,
	0x09, 0x24, 0x09, 0x25, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
	0xff, 0xff
};

static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_for_checksum, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_ENABLE_FOR_CHECKSUM, 0);

static char S6E3FAC_MU1S_AOD_SELF_MASK_RESTORE[] = {
	0x7A,
	0x05, 0x00, 0x00, 0x00, 0x09, 0x24, 0x09, 0x25,
	0x09, 0x26, 0x09, 0x27, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
	0xff, 0xff
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_self_mask_restore, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_SELF_MASK_RESTORE, 0);

static void *s6e3fac_mu1s_aod_self_mask_crc_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l2_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_crc_on1),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_dbist_on),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_disable),
	&DLYINFO(s6e3fac_mu1s_aod_self_mask_checksum_1frame_delay),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_sd_path),
	&DLYINFO(s6e3fac_mu1s_aod_self_spsram_sel_delay),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_crc_img_pkt),
	&DLYINFO(s6e3fac_mu1s_aod_self_spsram_write_delay),
	&PKTINFO(s6e3fac_mu1s_aod_reset_sd_path),
	&DLYINFO(s6e3fac_mu1s_aod_self_spsram_sel_delay),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_for_checksum),
	&DLYINFO(s6e3fac_mu1s_aod_self_mask_checksum_2frame_delay),
	&s6e3fac_dmptbl[DUMP_SELF_MASK_CRC],
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_restore),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_dbist_off),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_disable),
	&KEYINFO(s6e3fac_mu1s_aod_l2_key_disable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

// --------------------- end of check sum control ----------------------------

static char S6E3FAC_PARTIAL_MODE[] = {
	0x85,
	0x00, /* enable partial scan & enable partial hlpm*/
	0x00, /*enable partial hlpm area*/
	0x0F, 0x0F, 0x00, 0x00, 0x1F,
};
static DEFINE_PKTUI(s6e3fac_mu1s_aod_partial_mode, &s6e3fac_mu1s_aod_maptbl[SET_PARTIAL_MODE_MAPTBL], 0);
static DEFINE_VARIABLE_PACKET(s6e3fac_mu1s_aod_partial_mode, DSI_PKT_TYPE_WR, S6E3FAC_PARTIAL_MODE, 0);

static char S6E3FAC_PARTIAL_AREA[] = {
	0x85,
	0x00, 0x00, /* SCAN_G_SR */
	0x00, 0x00, /* SCAM_G_ER */
};
static DEFINE_PKTUI(s6e3fac_mu1s_aod_partial_area, &s6e3fac_mu1s_aod_maptbl[SET_PARTIAL_AREA_MAPTBL], 0);
static DEFINE_VARIABLE_PACKET(s6e3fac_mu1s_aod_partial_area, DSI_PKT_TYPE_WR, S6E3FAC_PARTIAL_AREA, 4);

static char S6E3FAC_PARTIAL_HLPM[] = {
	0x85,
	0x00, 0x00, /* AREA1_G_ER */
	0x00, 0x00, /* AREA2_G_ER */
	0x00, 0x00, /* AREA3_G_ER */
	0x00, 0x00, /* AREA4_G_ER */
};

static DEFINE_PKTUI(s6e3fac_mu1s_aod_partial_hlpm, &s6e3fac_mu1s_aod_maptbl[SET_PARTIAL_HLPM_MAPTBL], 0);
static DEFINE_VARIABLE_PACKET(s6e3fac_mu1s_aod_partial_hlpm, DSI_PKT_TYPE_WR, S6E3FAC_PARTIAL_HLPM, 8);

static void *s6e3fac_mu1s_aod_partial_enable_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_partial_hlpm),
	&PKTINFO(s6e3fac_mu1s_aod_partial_area),
	&PKTINFO(s6e3fac_mu1s_aod_partial_mode),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

static char S6E3FAC_DIS_PARTIAL_SCAN[] = {
	0x85,
	0x00,
};
DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_dis_partial_scan, DSI_PKT_TYPE_WR, S6E3FAC_DIS_PARTIAL_SCAN, 0);

static void *s6e3fac_mu1s_aod_partial_disable_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_dis_partial_scan),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

/* setting for aod porch */
static char S6E3FAC_MU1S_AOD_PORCH[] = {
	0x76,
	0x09, 0x0F
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_porch, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_PORCH, 0);

static char S6E3FAC_MU1S_AOD_ENABLE[] = {
	0x76,
	0x00, 0x11
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_enable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_ENABLE, 2);

static char S6E3FAC_MU1S_AOD_DISABLE[] = {
	0x76,
	0x00, 0x00
};
static DEFINE_STATIC_PACKET(s6e3fac_mu1s_aod_disable, DSI_PKT_TYPE_WR, S6E3FAC_MU1S_AOD_DISABLE, 2);

static void *s6e3fac_mu1s_aod_enter_aod_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_porch),
	&PKTINFO(s6e3fac_mu1s_aod_enable),
	&PKTINFO(s6e3fac_mu1s_aod_partial_hlpm),
	&PKTINFO(s6e3fac_mu1s_aod_partial_area),
	&PKTINFO(s6e3fac_mu1s_aod_partial_mode),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_disable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

static void *s6e3fac_mu1s_aod_exit_aod_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_disable),
	&PKTINFO(s6e3fac_mu1s_aod_dis_partial_scan),
	&KEYINFO(s6e3fac_mu1s_aod_l3_key_disable),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

static DEFINE_STATIC_PACKET_WITH_OPTION(s6e3fac_mu1s_aod_self_mask_img_pkt,
		DSI_PKT_TYPE_WR_SR, MU1S_SELF_MASK_IMG, 0, PKT_OPTION_SR_ALIGN_16);

static void *s6e3fac_mu1s_aod_self_mask_img_cmdtbl[] = {
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_enable),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_sd_path),
	&DLYINFO(s6e3fac_mu1s_aod_self_spsram_sel_delay),
	&PKTINFO(s6e3fac_mu1s_aod_self_mask_img_pkt),
	&DLYINFO(s6e3fac_mu1s_aod_self_spsram_write_delay),
	&PKTINFO(s6e3fac_mu1s_aod_reset_sd_path),
	&KEYINFO(s6e3fac_mu1s_aod_l1_key_disable),
};

static struct seqinfo s6e3fac_mu1s_aod_seqtbl[] = {
	SEQINFO_INIT(SELF_MASK_IMG_SEQ, s6e3fac_mu1s_aod_self_mask_img_cmdtbl),
	SEQINFO_INIT(SELF_MASK_ENA_SEQ, s6e3fac_mu1s_aod_self_mask_ena_cmdtbl),
	SEQINFO_INIT(SELF_MASK_DIS_SEQ, s6e3fac_mu1s_aod_self_mask_dis_cmdtbl),
	SEQINFO_INIT(ENTER_AOD_SEQ, s6e3fac_mu1s_aod_enter_aod_cmdtbl),
	SEQINFO_INIT(EXIT_AOD_SEQ, s6e3fac_mu1s_aod_exit_aod_cmdtbl),
	SEQINFO_INIT(ENABLE_PARTIAL_SCAN_SEQ, s6e3fac_mu1s_aod_partial_enable_cmdtbl),
	SEQINFO_INIT(DISABLE_PARTIAL_SCAN_SEQ, s6e3fac_mu1s_aod_partial_disable_cmdtbl),
	SEQINFO_INIT(SELF_MASK_CRC_SEQ, s6e3fac_mu1s_aod_self_mask_crc_cmdtbl),
};

static struct aod_tune s6e3fac_mu1s_aod = {
	.name = "s6e3fac_mu1s_aod",
	.nr_seqtbl = ARRAY_SIZE(s6e3fac_mu1s_aod_seqtbl),
	.seqtbl = s6e3fac_mu1s_aod_seqtbl,
	.nr_maptbl = ARRAY_SIZE(s6e3fac_mu1s_aod_maptbl),
	.maptbl = s6e3fac_mu1s_aod_maptbl,
	.self_mask_en = true,
};

#endif /* __S6E3FAC_MU1S_AOD_PANEL_H__ */
