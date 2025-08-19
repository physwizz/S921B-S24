#ifndef __EXYNOS_USB_TPMON_H__
#define __EXYNOS_USB_TPMON_H__

extern struct dwc3_exynos *g_dwc3_exynos;
extern struct dwc3_request *req;

void usb_tpmon_check_tp(void *data, struct dwc3_request *req);
void usb_tpmon_init_data(void);
void usb_tpmon_init(struct device *dev);
void usb_tpmon_exit(void);
void usb_tpmon_open(void);
void usb_tpmon_close(void);

#endif /* __EXYNOS_USB_TPMON_H__ */
