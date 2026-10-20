// SPDX-License-Identifier: GPL-2.0-only
/* Narrow Zen 5 desktop Tctl bridge for older kernels.
 * Temperature formula: Linux v6.12 drivers/hwmon/k10temp.c.
 * SMN access mechanism: Linux v5.15 arch/x86/kernel/amd_nb.c.
 * No thermal limits, fan controls, voltages or SMN data registers are written.
 */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/hwmon.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <asm/processor.h>
#include <asm/amd_nb.h>

struct zen5temp_data {
	struct pci_dev *root;
	struct mutex lock;
	bool shared_access;
	u16 node;
};

static int read_tctl(struct zen5temp_data *data, long *value)
{
	u32 raw, previous;
	int err, restore;

	if (data->shared_access) {
		err = amd_smn_read(data->node, 0x59800, &raw);
	} else {
		/* Used only when the old kernel has no cached AMD northbridges.
		 * Serialize this module's reads and restore the address selector.
		 * Do not run independent direct-SMN tools concurrently.
		 */
		mutex_lock(&data->lock);
		err = pci_read_config_dword(data->root, 0x60, &previous);
		if (!err) {
			err = pci_write_config_dword(data->root, 0x60, 0x59800);
			if (!err)
				err = pci_read_config_dword(data->root, 0x64, &raw);
			restore = pci_write_config_dword(data->root, 0x60, previous);
			if (!err)
				err = restore;
		}
		mutex_unlock(&data->lock);
	}
	if (err)
		return err < 0 ? err : -EIO;
	if (!raw || raw == ~0U)
		return -ENODATA;
	*value = (raw >> 21) * 125L;
	if ((raw & BIT(19)) || (raw & GENMASK(17, 16)) == GENMASK(17, 16))
		*value -= 49000;
	if (*value <= 0 || *value > 125000)
		return -ERANGE;
	return 0;
}

static umode_t zen5_visible(const void *data, enum hwmon_sensor_types type,
			   u32 attr, int channel)
{
	return type == hwmon_temp && channel == 0 &&
		(attr == hwmon_temp_input || attr == hwmon_temp_label) ? 0444 : 0;
}

static int zen5_read(struct device *dev, enum hwmon_sensor_types type,
		     u32 attr, int channel, long *value)
{
	if (type != hwmon_temp || attr != hwmon_temp_input || channel)
		return -EOPNOTSUPP;
	return read_tctl(dev_get_drvdata(dev), value);
}

static int zen5_label(struct device *dev, enum hwmon_sensor_types type,
		      u32 attr, int channel, const char **label)
{
	if (type != hwmon_temp || attr != hwmon_temp_label || channel)
		return -EOPNOTSUPP;
	*label = "Tctl";
	return 0;
}

static const struct hwmon_ops zen5_ops = {
	.is_visible = zen5_visible,
	.read = zen5_read,
	.read_string = zen5_label,
};
static const struct hwmon_channel_info *zen5_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT | HWMON_T_LABEL),
	NULL
};
static const struct hwmon_chip_info zen5_chip = {
	.ops = &zen5_ops,
	.info = zen5_info,
};

static void put_root(void *root)
{
	pci_dev_put(root);
}

static int zen5_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct zen5temp_data *data;
	struct device *hwmon;
	struct amd_northbridge *nb;
	long initial;
	int err, i;

	/* Deliberately limited to the user's single-socket Granite Ridge host. */
	if (boot_cpu_data.x86_vendor != X86_VENDOR_AMD ||
	    boot_cpu_data.x86 != 0x1a || boot_cpu_data.x86_model != 0x44 ||
	    pci_domain_nr(pdev->bus) != 0 || pdev->bus->number != 0 ||
	    pdev->devfn != PCI_DEVFN(0x18, 3))
		return -ENODEV;
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	mutex_init(&data->lock);
	data->root = pci_get_domain_bus_and_slot(0, 0, PCI_DEVFN(0, 0));
	if (!data->root)
		return -ENODEV;
	err = devm_add_action_or_reset(&pdev->dev, put_root, data->root);
	if (err)
		return err;
	if (data->root->vendor != PCI_VENDOR_ID_AMD || data->root->device != 0x14d8)
		return -ENODEV;
	/* Reuse the kernel's SMN lock whenever this host is already registered. */
	for (i = 0; i < amd_nb_num(); i++) {
		nb = node_to_amd_nb(i);
		if (nb && nb->root == data->root) {
			data->shared_access = true;
			data->node = i;
			break;
		}
	}
	if (!data->shared_access && amd_nb_num()) {
		dev_err(&pdev->dev, "Unknown existing SMN topology; refusing direct access\n");
		return -EBUSY;
	}
	err = read_tctl(data, &initial);
	if (err) {
		dev_err(&pdev->dev, "Tctl read failed: %d\n", err);
		return err;
	}
	hwmon = devm_hwmon_device_register_with_info(&pdev->dev, "zen5temp", data,
						   &zen5_chip, NULL);
	if (IS_ERR(hwmon))
		return PTR_ERR(hwmon);
	dev_info(&pdev->dev, "Tctl=%ld mC (%s SMN access)\n", initial,
		 data->shared_access ? "kernel" : "local serialized");
	return 0;
}

static const struct pci_device_id zen5_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_AMD, 0x14e3) },
	{ }
};
MODULE_DEVICE_TABLE(pci, zen5_ids);
static struct pci_driver zen5_driver = {
	.name = "zen5temp",
	.id_table = zen5_ids,
	.probe = zen5_probe,
};
module_pci_driver(zen5_driver);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Experimental Granite Ridge 1Ah/44h Tctl bridge for Linux 5.15");
