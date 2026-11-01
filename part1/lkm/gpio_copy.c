// Part 1, test 4: copy the input pin to the output pin inside a kernel module.

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>

static struct gpio_desc *inputPin;    // GPIO25, the encoder signal comes in here
static struct gpio_desc *outputPin;   // GPIO17, the copy goes out here


// The interrupt handler. Runs every time the input pin changes.
// Keep it short, and no printk in here (it is slow).
static irqreturn_t inputPinChanged(int irqNumber, void *deviceId)
{
	int inputValue = gpiod_get_value(inputPin);

	gpiod_set_value(outputPin, inputValue);

	return IRQ_HANDLED;
}


// Runs when the kernel finds our node in the device tree.
// Gets the two pins and connects the interrupt handler to the input pin.
static int gpioCopyProbe(struct platform_device *pdev)
{
	int irqNumber;

	// Get the input pin
	inputPin = devm_gpiod_get(&pdev->dev, "input", GPIOD_IN);
	if (IS_ERR(inputPin))
		return PTR_ERR(inputPin);

	// Get the output pin, starts low
	outputPin = devm_gpiod_get(&pdev->dev, "output", GPIOD_OUT_LOW);
	if (IS_ERR(outputPin))
		return PTR_ERR(outputPin);

	// Find the interrupt number that belongs to the input pin
	irqNumber = gpiod_to_irq(inputPin);
	if (irqNumber < 0)
		return irqNumber;

	pr_info("gpio_copy: copying input to output, IRQ %d\n", irqNumber);

	// Call inputPinChanged() on both edges (low->high and high->low)
	return devm_request_irq(&pdev->dev, irqNumber, inputPinChanged,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"gpio_copy", NULL);
}


// Runs on rmmod. Nothing to do: the devm_ functions give the pins and IRQ back by themselves.
static void gpioCopyRemove(struct platform_device *pdev)
{
	pr_info("gpio_copy: removed\n");
}


// Tells the kernel which device tree node is ours. Must match "compatible" in the overlay.
static const struct of_device_id gpioCopyMatchTable[] = {
	{ .compatible = "custom,gpio-copy" },
	{ }   // end marker
};
MODULE_DEVICE_TABLE(of, gpioCopyMatchTable);

static struct platform_driver gpioCopyDriver = {
	.probe  = gpioCopyProbe,
	.remove = gpioCopyRemove,
	.driver = {
		.name = "gpio-copy",
		.of_match_table = gpioCopyMatchTable,
	},
};


// insmod: register the driver, the kernel then calls gpioCopyProbe()
static int __init gpioCopyInit(void)
{
	return platform_driver_register(&gpioCopyDriver);
}

// rmmod: unregister the driver, the kernel then calls gpioCopyRemove()
static void __exit gpioCopyExit(void)
{
	platform_driver_unregister(&gpioCopyDriver);
}

module_init(gpioCopyInit);
module_exit(gpioCopyExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eirikur");
MODULE_DESCRIPTION("Copies a GPIO input pin to an output pin using an interrupt");
MODULE_VERSION("0.1");
