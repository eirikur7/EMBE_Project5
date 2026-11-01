// Part 2: encoder counter inside a kernel module.
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#define DEVICE_NAME "encoder"   // name in /proc/devices, node is /dev/encoder

static struct gpio_desc *pinC1;   // GPIO25, encoder C1
static struct gpio_desc *pinC2;   // GPIO24, encoder C2

// The count. Changed in the interrupt, read by read(), so it is atomic
// (it can never be read half-way through a change).
static atomic_t encoderCount = ATOMIC_INIT(0);

static int majorNumber;


// The interrupt handler. Runs on every edge of C1 (rising and falling).
// Same rule as in Project 4: C2 different from C1 = one way, the same = the other way.
// Keep it short, and no printk in here (it is slow).
static irqreturn_t c1Changed(int irqNumber, void *deviceId)
{
	int valueC1 = gpiod_get_value(pinC1);
	int valueC2 = gpiod_get_value(pinC2);

	if (valueC2 != valueC1)
		atomic_inc(&encoderCount);
	else
		atomic_dec(&encoderCount);

	return IRQ_HANDLED;
}


// Runs when a program calls read() on /dev/encoder.
// Gives the count as a binary int (sizeof(int) = 4 bytes).
static ssize_t encoderRead(struct file *filep, char __user *userBuffer,
			   size_t length, loff_t *offset)
{
	int count = atomic_read(&encoderCount);

	// The program must ask for at least 4 bytes
	if (length < sizeof(count))
		return -EINVAL;

	// Copy the count to the program (it lives in user space, so we must use copy_to_user)
	if (copy_to_user(userBuffer, &count, sizeof(count)))
		return -EFAULT;

	return sizeof(count);
}

static const struct file_operations encoderFileOperations = {
	.owner = THIS_MODULE,
	.read  = encoderRead,
};


// Runs when the kernel finds our node in the device tree.
// Gets the pins, connects the interrupt, and registers /dev/encoder.
static int encoderProbe(struct platform_device *pdev)
{
	int irqNumber;
	int result;

	// Get the two encoder pins
	pinC1 = devm_gpiod_get(&pdev->dev, "c1", GPIOD_IN);
	if (IS_ERR(pinC1))
		return PTR_ERR(pinC1);

	pinC2 = devm_gpiod_get(&pdev->dev, "c2", GPIOD_IN);
	if (IS_ERR(pinC2))
		return PTR_ERR(pinC2);

	// Find the interrupt number that belongs to C1
	irqNumber = gpiod_to_irq(pinC1);
	if (irqNumber < 0)
		return irqNumber;

	// Call c1Changed() on both edges of C1
	result = devm_request_irq(&pdev->dev, irqNumber, c1Changed,
				  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				  "encoder", NULL);
	if (result)
		return result;

	// Register the character device (0 = the kernel picks a free major number)
	majorNumber = register_chrdev(0, DEVICE_NAME, &encoderFileOperations);
	if (majorNumber < 0)
		return majorNumber;

	pr_info("encoder: ready, IRQ %d, major number %d\n", irqNumber, majorNumber);
	return 0;
}


// Runs on rmmod. The devm_ functions give the pins and IRQ back by themselves.
static void encoderRemove(struct platform_device *pdev)
{
	unregister_chrdev(majorNumber, DEVICE_NAME);
	pr_info("encoder: removed\n");
}


// Tells the kernel which device tree node is ours. Must match "compatible" in the overlay.
static const struct of_device_id encoderMatchTable[] = {
	{ .compatible = "custom,encoder" },
	{ }   // end marker
};
MODULE_DEVICE_TABLE(of, encoderMatchTable);

static struct platform_driver encoderDriver = {
	.probe  = encoderProbe,
	.remove = encoderRemove,
	.driver = {
		.name = "encoder",
		.of_match_table = encoderMatchTable,
	},
};


// insmod: register the driver, the kernel then calls encoderProbe()
static int __init encoderInit(void)
{
	return platform_driver_register(&encoderDriver);
}

// rmmod: unregister the driver, the kernel then calls encoderRemove()
static void __exit encoderExit(void)
{
	platform_driver_unregister(&encoderDriver);
}

module_init(encoderInit);
module_exit(encoderExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eirikur");
MODULE_DESCRIPTION("Quadrature encoder counter, read the count from /dev/encoder");
MODULE_VERSION("0.1");
