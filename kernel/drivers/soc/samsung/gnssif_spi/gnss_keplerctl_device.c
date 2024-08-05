#include <linux/init.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#include "include/gnss.h"
#include "gnss_prj.h"
#include "gnss_utils.h"

/* handling interrupt by BETP messages from kepler through SPI */
static irqreturn_t kepler_spi_irq_handler(int irq, void *data)
{
	struct gnss_ctl *gc = (struct gnss_ctl *)data;
	struct io_device *iod = gc->iod;
	struct link_device *ld = iod->ld;

	if (!gc->iod) {
		gif_err("probe is not done yet\n");
		gif_disable_irq_nosync(&gc->irq_gnss2ap_spi);
		return IRQ_HANDLED;
	}

	gif_debug("received SPI interrupt from GNSS\n");

	gif_disable_irq_nosync(&gc->irq_gnss2ap_spi);

	if (atomic_read(&gc->wait_rdy)) {
		gif_debug("gnss_rdy_cmpl\n");
		atomic_set(&gc->wait_rdy, 0);
		complete_all(&gc->gnss_rdy_cmpl);
		return IRQ_HANDLED;
	}

	if (atomic_read(&gc->rx_in_progress) == 0) {
		gif_debug("run workq\n");
		queue_delayed_work(ld->rx_wq, &ld->rx_dwork, 0);
	}

	return IRQ_HANDLED;
}

static int kepler_spi_status(struct gnss_ctl *gc)
{
	return gpio_get_value(gc->gpio_gnss2ap_spi.num);
}

static void kepler_send_betp_int(struct gnss_ctl *gc, int value)
{
	gif_debug("About to Set ap2gnss_spi value to %d\n", value);
	gpio_set_value(gc->gpio_ap2gnss_spi.num, value);
	gif_debug("complete to set ap2gnss_spi value to %d\n", value);
}

static int kepler_betp_int_status(struct gnss_ctl *gc)
{
	return gpio_get_value(gc->gpio_ap2gnss_spi.num);
}

static void gnss_get_ops(struct gnss_ctl *gc)
{
	gc->ops.gnss_spi_status = kepler_spi_status;
	gc->ops.gnss_send_betp_int = kepler_send_betp_int;
	gc->ops.gnss_betp_int_status = kepler_betp_int_status;
}

static int init_ctl_device(struct gnss_ctl *gc, struct gnss_pdata *pdata)
{
	int ret = 0;
	struct platform_device *pdev = NULL;
	struct device_node *np;

	gif_info("Initializing GNSS control device\n");

	gnss_get_ops(gc);

	dev_set_drvdata(gc->dev, gc);

	pdev = to_platform_device(gc->dev);
	np = pdev->dev.of_node;

	/* GNSS2AP SPI INT */
	gc->gpio_gnss2ap_spi.num = of_get_named_gpio(np, "gpio_gnss2ap_spi", 0);
	if (gc->gpio_gnss2ap_spi.num < 0) {
		gif_err("Can't get gpio_gnss2ap_spi!\n");
		return -ENODEV;
	}
	gc->gpio_gnss2ap_spi.label = "GNSS2AP_SPI";
	gpio_request(gc->gpio_gnss2ap_spi.num, gc->gpio_gnss2ap_spi.label);
	gc->irq_gnss2ap_spi.num = gpio_to_irq(gc->gpio_gnss2ap_spi.num);
	gif_init_irq(&gc->irq_gnss2ap_spi, gc->irq_gnss2ap_spi.num,
			"kepler_spi_irq_handler", IRQF_TRIGGER_HIGH);

	ret = gif_request_irq(&gc->irq_gnss2ap_spi, kepler_spi_irq_handler, gc);
	if (ret) {
		gif_err("Request irq fail - kepler_spi_irq_handler(%d)\n", ret);
		return ret;
	}

	gc->gpio_ap2gnss_spi.num =
				of_get_named_gpio(np, "gpio_ap2gnss_spi", 0);
	if (gc->gpio_ap2gnss_spi.num < 0) {
		gif_err("Can't get gpio_ap2gnss_spi!\n");
		return -ENODEV;
	}
	gc->gpio_ap2gnss_spi.label = "AP2GNSS_SPI";
	gpio_request(gc->gpio_ap2gnss_spi.num, gc->gpio_ap2gnss_spi.label);
	gpio_direction_output(gc->gpio_ap2gnss_spi.num, 0);

	/* initialize gpio values to all zero */
	gpio_set_value(gc->gpio_ap2gnss_spi.num, 0);

	init_completion(&gc->gnss_rdy_cmpl);

	atomic_set(&gc->wait_rdy, 0);
	atomic_set(&gc->tx_in_progress, 0);
	atomic_set(&gc->rx_in_progress, 0);

	return ret;
}

struct gnss_ctl *create_ctl_device(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gnss_pdata *pdata = pdev->dev.platform_data;
	struct gnss_ctl *gc;
	int ret;

	gif_info("+++\n");
	gc = devm_kzalloc(dev, sizeof(struct gnss_ctl), GFP_KERNEL);
	if (!gc) {
		gif_err("%s: gc devm_kzalloc fail\n", pdata->name);
		return NULL;
	}
	gc->dev = dev;
	gc->pdata = pdata;
	gc->name = pdata->name;

	ret = init_ctl_device(gc, pdata);
	if (ret) {
		gif_err("%s: init_ctl_device fail (err %d)\n",
				pdata->name, ret);
		devm_kfree(dev, gc);
		return NULL;
	}

	gif_info("---\n");

	return gc;
}
