/*
 * Copyrights (C) 2017 Samsung Electronics, Inc.
 * Copyrights (C) 2017 Maxim Integrated Products, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_MFD_MAX77775_H
#define __LINUX_MFD_MAX77775_H

#define MAX77775_CCPD_NAME	"MAX77775"

#undef  __CONST_FFS
#define __CONST_FFS(_x) \
	((_x) & 0x0F ? \
	 ((_x) & 0x03 ? ((_x) & 0x01 ? 0 : 1) : ((_x) & 0x04 ? 2 : 3)) : \
	 ((_x) & 0x30 ? ((_x) & 0x10 ? 4 : 5) : ((_x) & 0x40 ? 6 : 7)))

#undef FFS
#define FFS(_x) \
	((_x) ? __CONST_FFS(_x) : 0)

#undef  BIT_RSVD
#define BIT_RSVD  0

#undef  BITS
#define BITS(_end, _start) \
	((BIT(_end) - BIT(_start)) + BIT(_end))

#undef  __BITS_GET
#define __BITS_GET(_word, _mask, _shift) \
	(((_word) & (_mask)) >> (_shift))

#undef  BITS_GET
#define BITS_GET(_word, _bit) \
	__BITS_GET(_word, _bit, FFS(_bit))

#undef  __BITS_SET
#define __BITS_SET(_word, _mask, _shift, _val) \
	(((_word) & ~(_mask)) | (((_val) << (_shift)) & (_mask)))

#undef  BITS_SET
#define BITS_SET(_word, _bit, _val) \
	__BITS_SET(_word, _bit, FFS(_bit), _val)

#undef  BITS_MATCH
#define BITS_MATCH(_word, _bit) \
	(((_word) & (_bit)) == (_bit))

/*
 * Register address
 */
#define	REG_PRODUCT_ID			0x10 /* replaced address */
#define	REG_UIC_FW_REV			0x01

#define	REG_UIC_INT				0x02
#define	REG_CC_INT				0x03
#define	REG_PD_INT				0x04
#define	REG_VDM_INT				0x05
#define	REG_SPARE_INT			0x06
#define	REG_USBC_STATUS1		0x08
#define	REG_USBC_STATUS2		0x09
#define	REG_BC_STATUS			0x0A
#define	REG_UIC_FW_REV2			0x0B
#define	REG_CC_STATUS1			0x0C
#define	REG_CC_STATUS2			0x0D
#define	REG_PD_STATUS1			0x0E
#define	REG_PD_STATUS2			0x0F
#define	REG_UIC_INT_M			0x12
#define	REG_CC_INT_M			0x13
#define	REG_PD_INT_M			0x14
#define	REG_VDM_INT_M			0x15
#define REG_SPARE_INT_M			0x16

#define REG_OPCODE				0x21
#define REG_OPCODE_DATA			0x22
#define REG_OPCDE_RES			0x51

#define REG_END_DATA			0x41

/*
 * REG_INT_M Initial values
 */
#define REG_UIC_INT_M_INIT		0x04
#define REG_CC_INT_M_INIT		0x20
#define REG_PD_INT_M_INIT		0x00
#define REG_VDM_INT_M_INIT		0xF0
#define REG_SPARE_INT_M_INIT	0x7F

/*
 * REG_UIC_INT Interrupts
 */
#define BIT_APCmdResI			BIT(7)
#define BIT_SYSMsgI				BIT(6)
#define BIT_VBUSDetI			BIT(5)
#define BIT_VbADCI				BIT(4)
#define BIT_DCDTmoI				BIT(3)
#define BIT_CHGTypI				BIT(1)
#define BIT_UIDADCI				BIT(0)

/*
 * REG_CC_INT Interrupts
 */
#define BIT_VCONNOCPI			BIT(7)
#define BIT_VSAFE0VI			BIT(6)
#define BIT_DETABRTI			BIT(5)
#define BIT_VCONNSCI			BIT(4)
#define BIT_CCPinStatI			BIT(3)
#define	BIT_CCIStatI			BIT(2)
#define	BIT_CCVcnStatI			BIT(1)
#define	BIT_CCStatI				BIT(0)

/*
 * REG_PD_INT Interrupts
 */
#define BIT_PDMsgI				BIT(7)
#define BIT_DataRole			BIT(5)
#define BIT_SSAccI				BIT(1)
#define BIT_FCTIDI				BIT(0)

/*
 * REG_SPARE_INT Interrupts
 */
#define BIT_USBID				BIT(7)

/*
 * REG_USBC_STATUS1
 */
#define BIT_VBADC				BITS(7, 4)
#define BIT_UIDADC				BITS(2, 0)

/*
 * REG_USBC_STATUS2
 */
#define BIT_SYSMsg				BITS(7, 0)

/*
 * REG_BC_STATUS
 */
#define BIT_VBUSDet				BIT(7)
#define BIT_PrChgTyp			BITS(5, 3)
#define BIT_DCDTmo				BIT(2)
#define BIT_ChgTyp				BITS(1, 0)

/*
 * REG_CC_STATUS1
 */
#define BIT_CCPinStat			BITS(7, 6)
#define BIT_CCIStat				BITS(5, 4)
#define BIT_CCVcnStat			BIT(3)
#define BIT_CCStat				BITS(2, 0)

/*
 * REG_CC_STATUS2
 */
#define BIT_CCSBUSHORT			BITS(7, 6)
#define BIT_VCONNOCP			BIT(5)
#define BIT_VCONNSC				BIT(4)
#define BIT_VSAFE0V				BIT(3)
#define BIT_DETABRT				BIT(2)
#define BIT_ConnStat			BIT(1)
#define BIT_Altmode				BIT(0)

/*
 * REG_PD_STATUS1
 */
#define BIT_PDMsg				BITS(7, 0)

/*
 * REG_PD_STATUS2
 */
#define BIT_PD_DataRole			BIT(7)
#define BIT_PD_ENTER_MODE		BIT(5)
#define BIT_PD_PSRDY			BIT(4)
#define BIT_FCT_ID				BITS(3, 0)


/** opcode reg **/

/*
 * CC Control1 Write
 */
#define BIT_CCVcnEn				BIT(7)
#define BIT_CCTrySnkEn			BIT(6)
#define BIT_CCSrcDbgEn			BIT(4)
#define BIT_CCSnkDbgEn			BIT(3)
#define BIT_CCAudEn				BIT(2)
#define BIT_CCSrcSnk			BIT(1) // CCSrcEn
#define BIT_CCSnkSrc			BIT(0) // CCSnkEn

/*
 * max77766 role
 */
enum max77775_data_role {
	UFP = 0,
	DFP,
};
enum max77775_power_role {
	SNK = 0,
	SRC,
};
enum max77775_vcon_role {
	OFF = 0,
	ON
};

/*
 * F/W update
 */
#define FW_CMD_READ			0x3
#define FW_CMD_READ_SIZE	6	/* cmd(1) + len(1) + data(4) */

#define FW_CMD_WRITE		0x1
#define FW_CMD_WRITE_SIZE	36	/* cmd(1) + len(1) + data(34) */

#define FW_CMD_END			0x0

#define FW_HEADER_SIZE		8
#define FW_VERIFY_DATA_SIZE 3

#define FW_VERIFY_TRY_COUNT 10
#define FW_SECURE_MODE_TRY_COUNT 10

#define FW_WAIT_TIMEOUT			(1000 * 5) /* 5 sec */
#define I2C_SMBUS_BLOCK_HALF	(I2C_SMBUS_BLOCK_MAX / 2)

#define GET_CCCTRL4_LOCK_ERROR_EN(_x)		((_x & (0x1 << 4)) >> 1)

enum {
	NO_DETERMINATION = 0,
	CC1_ACTIVE,
	CC2_ACTVIE,
	AUDIO_ACCESSORY,
	DEBUG_ACCESSORY,
	ERROR,
	DISABLED,
	DEBUG_SNK,
};

enum {
	NOT_IN_UFP_MODE = 0,
	CCI_500mA,
	CCI_1_5A,
	CCI_3_0A,
	CCI_SHORT,
};

/*
 * All type of Interrupts
 */
enum {
	AP_Command_respond = 7,
	USBC_System_message = 6,
	CHGIN_Voltage = 5,
	CHGIN_Voltage_ADC = 4,
	DCD_Timer = 3,
	Charge_Type = 1,
	UID_ADC = 0,
};

enum max77775_chg_type {
	CHGTYP_NOTHING = 0,
	CHGTYP_USB_SDP,
	CHGTYP_CDP_T,
	CHGTYP_DCP,
};

enum max77775_pr_chg_type {
	Unknown = 0,
	Samsung_2A,
	Apple_05A,
	Apple_1A,
	Apple_2A,
	Apple_12W,
	DCP_3A,
	RFU_CHG,
};

enum max77775_ccpd_device {
	DEV_NONE = 0,
	DEV_OTG,

	DEV_USB,
	DEV_CDP,
	DEV_DCP,

	DEV_APPLE500MA,
	DEV_APPLE1A,
	DEV_APPLE2A,
	DEV_APPLE12W,
	DEV_DCP3A,
	DEV_HVDCP,
	DEV_QC,

	DEV_FCT_GND,
	DEV_FCT_1K,
	DEV_FCT_56K,
	DEV_FCT_255K,
	DEV_FCT_301K,
	DEV_FCT_523K,
	DEV_FCT_619K,
	DEV_FCT_OPEN,

	DEV_PD_TA,
	DEV_PD_AMA,

	DEV_UNKNOWN,
};

enum max77775_uidadc {
	UID_GND	= 0,
	UID_100Kohm = 1,
	UID_255Kohm = 3,
	UID_301Kohm	= 4,
	UID_523Kohm = 5,
	UID_619Kohm = 6,
	UID_Open = 7,
};

enum max77775_fctid {
	FCT_GND = 1,
	FCT_56Kohm = 2,
	FCT_255Kohm = 3,
	FCT_301Kohm = 4,
	FCT_523Kohm = 5,
	FCT_619Kohm = 6,
	FCT_OPEN = 7,
};

enum max77775_cc_pin_state {
	cc_No_Connection = 0,
	cc_SINK,
	cc_SOURCE,
	cc_Audio_Accessory,
	cc_Debug_Accessory,
	cc_Error,
	cc_Disabled,
	cc_Debug_Sink,
};

enum max77775_usbc_SYSMsg {
	SYSERROR_NONE = 0x00,
	/*Reserved = 0x01,*/
	/*Reserved = 0x02,*/
	SYSERROR_BOOT_WDT = 0x03,
	SYSERROR_BOOT_SWRSTREQ = 0x04,
	SYSMSG_BOOT_POR = 0x05,

	SYSERROR_HV_NOVBUS = 0x10,
	SYSERROR_HV_FMETHOD_RXPERR = 0x11,
	SYSERROR_HV_FMETHOD_RXBUFOW = 0x12,
	SYSERROR_HV_FMETHOD_RXTFR = 0x13,
	SYSERROR_HV_FMETHOD_MPNACK = 0x14,
	SYSERROR_HV_FMETHOD_RESET_FAIL = 0x15,

	SYSMsg_AFC_Done = 0x20,

	SYSERROR_SYSPOS = 0x30,
	SYSERROR_APCMD_UNKNOWN = 0x31,
	SYSERROR_APCMD_INPROGRESS = 0x32,
	SYSERROR_APCMD_FAIL = 0x33,

	SYSMSG_CCx_5V_AFC_SHORT = 0x61,
	SYSMSG_SBUx_GND_SHORT = 0x62,
	SYSMSG_CCx_5V_SHORT = 0x65,
	SYSMSG_SBUx_5V_SHORT = 0x66,

	SYSERROR_FACTORY_RID0 = 0x70,
	SYSERROR_POWER_NEGO = 0x80,
	SYSERROR_CCRP_HIGH = 0x90, /* PD Charger Connected while Water state */
	SYSERROR_CCRP_LOW = 0x91, /* PD Charger Disconnected while Water state */

	SYSMSG_RELEASE_CC1_SHORT = 0xB3,
	SYSMSG_RELEASE_CC2_SHORT = 0xB4,

	SYSMSG_ABNORMAL_TA = 0xC1,
};

enum max77775_pdmsg {
	Nothing_happened = 0x00,
	Sink_PD_PSRdy_received = 0x01,
	Sink_PD_Error_Recovery = 0x02,
	Sink_PD_SenderResponseTimer_Timeout	= 0x03,
	Source_PD_PSRdy_Sent = 0x04,
	Source_PD_Error_Recovery = 0x05,
	Source_PD_SenderResponseTimer_Timeout = 0x06,
	PD_DR_Swap_Request_Received	= 0x07,
	PD_PR_Swap_Request_Received	= 0x08,
	PD_VCONN_Swap_Request_Received = 0x09,
	Received_PD_Message_in_illegal_state = 0x0A,
	SRC_CAP_RECEIVED = 0x0B,

	Samsung_Accessory_is_attached = 0x10,
	VDM_Attention_message_Received = 0x11,
	Rejcet_Received = 0x12,
	Not_Supported_Received = 0x13,
	Prswap_Snktosrc_Sent	= 0x14,
	Prswap_Srctosnk_Sent	= 0x15,
	HARDRESET_RECEIVED = 0x16,
	Get_Vbus_turn_on = 0x17,
	Get_Vbus_turn_off = 0x18,
	HARDRESET_SENT = 0x19,
	PRSWAP_SRCTOSWAP = 0x1A,
	PRSWAP_SWAPTOSNK = 0X1B,
	PRSWAP_SNKTOSWAP = 0x1C,
	PRSWAP_SWAPTOSRC = 0x1D,

	Sink_PD_Disabled = 0x20,
	Source_PD_Disabled = 0x21,
	Current_Cable_Connected = 0x22,

	Get_Source_Capabilities_Extended_Received = 0x30,
	Get_Status_Received = 0x31,
	Get_Battery_Cap_Received = 0x32,
	Get_Battery_Status_Received = 0x33,
	Get_Manufacturer_Info_Received = 0x34,
	Source_Capabilities_Extended_Received = 0x35,
	Status_Received = 0x36,
	Battery_Capabilities_Received = 0x37,
	Batery_Status_Received = 0x38,
	Manufacturer_Info_Received = 0x39,
	Alert_Message = 0x3e,
	VDM_NAK_Recevied = 0x40,
	VDM_BUSY_Recevied = 0x41,
	VDM_ACK_Recevied = 0x42,
	VDM_REQ_Recevied = 0x43,

	AFC_Sink_PD_Capabilities_Received = 0x50,
	AFC_Sink_PD_PSRdy_Received = 0x51,
	AFC_VDM_ResponseTimer_Timout = 0x52,

	PDMSG_DP_DISCOVER_IDENTITY = 0x61,
	PDMSG_DP_DISCOVER_SVID = 0x62,
	PDMSG_DP_DISCOVER_MODES = 0x63,
	PDMSG_DP_ENTER_MODE = 0x64,
	PDMSG_DP_EXIT_MODE = 0x65,
	PDMSG_DP_STATUS = 0x66,
	PDMSG_DP_CONFIGURE = 0x67,
	PDMSG_DP_ATTENTION = 0x68,

	PDMSG_SRC_ACCEPT = 0x70,

};
enum max77775_connstat {
	DRY = 0x00,
	WATER = 0x01,
};

/*
 * External type definition
 */
#define MAX77775_AUTOIBUS_FW_AT_OFF	3
#define MAX77775_AUTOIBUS_FW_OFF	2
#define MAX77775_AUTOIBUS_AT_OFF	1
#define MAX77775_AUTOIBUS_ON	0

#define OPCODE_WAIT_TIMEOUT (3000) /* 3000ms */

#define OPCODE_WRITE_COMMAND 0x21
#define OPCODE_READ_COMMAND 0x51
#define OPCODE_SIZE 1
#define OPCODE_HEADER_SIZE 1
#define OPCODE_DATA_LENGTH 32
#define OPCODE_MAX_LENGTH (OPCODE_DATA_LENGTH + OPCODE_SIZE)
#define OPCODE_WRITE 0x21
#define OPCODE_DATAOUT1 0x22
#define OPCODE_WRITE_END 0x41
#define OPCODE_READ 0x51
#define OPCODE_WRITE_SEQ 0x1
#define OPCODE_READ_SEQ 0x2
#define OPCODE_RW_SEQ 0x3
#define OPCODE_PUSH_SEQ 0x4
#define OPCODE_UPDATE_SEQ 0x5

typedef enum {
	OPCODE_BCCTRL1_R = 0x01,
	OPCODE_BCCTRL1_W,
	OPCODE_BCCTRL2_R,
	OPCODE_BCCTRL2_W,
	OPCODE_CTRL1_R = 0x05,
	OPCODE_CTRL1_W,

	OPCODE_CCCTRL1_R = 0x0B,
	OPCODE_CCCTRL1_W,
	OPCODE_CCCTRL2_R,
	OPCODE_CCCTRL2_W,
	OPCODE_CCCTRL3_R,
	OPCODE_CCCTRL3_W,
	OPCODE_CCCTRL4_R = 0x11,
	OPCODE_CCCTRL4_W,
	OPCODE_VCONN_ILIM_R = 0x13,
	OPCODE_VCONN_ILIM_W,

	OPCODE_CCCTRL5_W = 0x16,

	OPCODE_AFC_HV_W = 0x20,
	OPCODE_AFC_HV_RESULT_W = 0x21,
	OPCODE_AFC_QC_2_0_SET = 0x22,

	OPCODE_CHGIN_ILIM_R = 0x25,
	OPCODE_CHGIN_ILIM_W,

	OPCODE_USB_ID_SET = 0x27,
	OPCODE_READ_SBU = 0x28,
	OPCODE_CONTROL_JIG_R = 0x29,
	OPCODE_CONTROL_JIG_W = 0x2A,
	OPCODE_ICURR_AUTOIBUS_ON = 0x2C,

	OPCODE_SET_SNKCAP = 0x2E,
	OPCODE_READ_CC = 0x2D,
	OPCODE_CURRENT_SRCCAP = 0x30,
	OPCODE_GET_SRCCAP = 0x31,
	OPCODE_SRCCAP_REQUEST = 0x32,
	OPCODE_SET_SRCCAP = 0x33,
	OPCODE_SEND_GET_REQUEST = 0x34,
	OPCODE_READ_RESPONSE_FOR_GET_REQUEST = 0x35,
	OPCODE_SWAP_REQUEST = 0x37,
	OPCODE_APDO_SRCCAP_REQUEST = 0x3A,
	OPCODE_SET_PPS = 0x3C,

	OPCODE_VDM_DISCOVER_SET_VDM_REQ = 0x48,
	OPCODE_VDM_DISCOVER_GET_VDM_RESP = 0x4B,

	OPCODE_SAMSUNG_FACTORY_TEST = 0x54,
	OPCODE_SET_ALTERNATEMODE = 0x55,
	OPCODE_SAMSUNG_CCSBU_SHORT = 0x56,
	OPCODE_SAMSUNG_FW_AUTOIBUS = 0x57,

	OPCODE_SAMSUNG_READ_MESSAGE = 0x5D,
	OPCODE_SNK_SELECTED_PDO = 0x65,
	OPCODE_FW_OPCODE_CLEAR = 0x70,
	OPCODE_SBU_CTRL1_R = 0x85,
	OPCODE_SBU_CTRL1_W = 0x86,
	OPCODE_FCCTRL1_W = 0x88,
	OPCODE_ACTIVE_DISCHARGE = 0x91,
	OPCODE_HICCUP_ENABLE = 0x92,
	OPCODE_AUTO_SHIPMODE = 0x97,
	OPCODE_BYPASS_MTN = 0x98,
	OPCODE_CHGRCV_RAMP = 0x9F,
	OPCODE_NONE = 0xff,
} max77775_opcode_list;

typedef enum{
	OPCODE_ID_VDM_DISCOVER_IDENTITY = 0x1,
	OPCODE_ID_VDM_DISCOVER_SVIDS = 0x2,
	OPCODE_ID_VDM_DISCOVER_MODES = 0x3,
	OPCODE_ID_VDM_ENTER_MODE = 0x4,
	OPCODE_ID_VDM_EXIT_MODE = 0x5,
	OPCODE_ID_VDM_ATTENTION = 0x6,
	OPCODE_ID_VDM_SVID_DP_STATUS = 0x10,
	OPCODE_ID_VDM_SVID_DP_CONFIGURE = 0x11,
} max77775_vdm_list;

enum{
	OPCODE_GET_SRC_CAP_EXT = 0,
	OPCODE_GET_STATUS,
	OPCODE_GET_BAT_CAP,
	OPCODE_GET_BAT_STS,
	OPCODE_GET_MANUFACTURE_INFO,
};

typedef enum {
	OPCODE_WAIT_START = 0,
	OPCODE_WAIT_END,
} max77775_opcode_wait_type;


typedef enum {
	OPCODE_NOTI_START = 0x0,
	OPCODE_NOTI_NONE = 0xff,
} max77775_opcode_noti_cmd;


/* SAMSUNG OPCODE */
#define REG_NONE 0xff
#define CCIC_IRQ_INIT_DETECT		(-1)

//#define MINOR_VERSION_MASK 0b00000111

/* PRODUCT ID  */
#define FW_PRODUCT_ID_REG 3
//#define STAR_PRODUCT_ID 0b0000
//#define Lykan_PRODUCT_ID 0b0001
//#define BEYOND_PRODUCT_ID 0b0010
#endif


