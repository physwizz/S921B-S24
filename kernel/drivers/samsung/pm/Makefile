# drivers/samsung/pm/Makefile
#
# Copyright (c) 2010-2021 Samsung Electronics Co., Ltd.
#		http://www.samsung.com/
#
# Licensed under GPLv2

obj-y				:=
obj-m				:=
obj-n				:=
obj-				:=

# Samsung Mobile Power Management Debugging Support
obj-$(CONFIG_SEC_PM_DEBUG)		+= sec_pm_debug.o
obj-$(CONFIG_SEC_PM_THERMISTOR)		+= sec_thermistor.o
obj-$(CONFIG_SEC_PM_SMPL_WARN)		+= sec_pm_smpl_warn.o

ccflags-y := -Wformat
