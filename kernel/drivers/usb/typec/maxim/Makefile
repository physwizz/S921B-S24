#
# Makefile for ccic devices
#
subdir-ccflags-y := -Wformat
obj-$(CONFIG_CCIC_MAX77705)		+= pdic_max77705.o
pdic_max77705-y		:= max77705_cc.o max77705_pd.o max77705_usbc.o max77705_alternate.o
pdic_max77705-$(CONFIG_CCIC_MAX77705_DEBUG) += max77705_debug.o
pdic_max77705-$(CONFIG_MUIC_MAX77705) += max77705-muic.o
pdic_max77705-$(CONFIG_HV_MUIC_MAX77705_AFC) += max77705-muic-afc.o
pdic_max77705-$(CONFIG_MUIC_MAX77705_PDIC) += max77705-muic-ccic.o

#
# Makefile for ccic devices
#
subdir-ccflags-y := -Wformat
obj-$(CONFIG_CCIC_MAX77775)		+= pdic_max77775.o
pdic_max77775-y		:= max77775_cc.o max77775_pd.o max77775_usbc.o max77775_alternate.o
pdic_max77775-$(CONFIG_MUIC_MAX77775) += max77775-muic.o max77775-muic-ccic.o
pdic_max77775-$(CONFIG_HV_MUIC_MAX77775_AFC) += max77775-muic-afc.o

GCOV_PROFILE_max77775_alternate.o		:= $(CONFIG_SEC_KUNIT)
