/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <kernel.h>
#include <math.h>
#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <drivers/sensor.h>

#define MOBILE 2

struct ble_data{
    int8_t dev;
    int8_t indic;
    int32_t data;
};

static struct ble_data ble;


static uint8_t simulate_ble;

static struct bt_uuid_128 ble_uuid = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static struct bt_uuid_16 ble_read_uuid = BT_UUID_INIT_16(0xf1de);

bool static_node = false;

static void ble_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate_ble = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_ble(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(ble));
}

/* define gatt service */
BT_GATT_SERVICE_DEFINE(ble_svc,
	BT_GATT_PRIMARY_SERVICE(&ble_uuid),
	BT_GATT_CHARACTERISTIC(&ble_read_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, 
	        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_ble, NULL, &ble),
	BT_GATT_CCC(ble_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.cancel = auth_cancel,
};


static void fetch_and_display(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	} else {
		for(int i = 0; i < 3; i++){
		    ble.data = accel[i].val1;
		    ble.indic = i * 2;
		    bt_gatt_notify(NULL, &ble_svc.attrs[2], &ble, sizeof(ble));
		    k_sleep(K_MSEC(1));
		    ble.data = accel[i].val2;
		    ble.indic = i * 2 + 1;
		    bt_gatt_notify(NULL, &ble_svc.attrs[2], &ble, sizeof(ble));
		    k_sleep(K_MSEC(1));
		}
	}
}

#ifdef CONFIG_LIS2DH_TRIGGER
static void trigger_handler(const struct device *dev,
			    struct sensor_trigger *trig)
{
	fetch_and_display(dev);
}
#endif


void main(void)
{
	int err;
	err = bt_enable(NULL);
    
    bt_ready();

	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);
    
    ble.dev = MOBILE;
    
    
	/*********** sensor sample code ***********/
	const struct device *sensor = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));

	if (sensor == NULL) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, st_lis2dh)));
		return;
	}

#if CONFIG_LIS2DH_TRIGGER
	{
		struct sensor_trigger trig;
		int rc;

		trig.type = SENSOR_TRIG_DATA_READY;
		trig.chan = SENSOR_CHAN_ACCEL_XYZ;

		if (IS_ENABLED(CONFIG_LIS2DH_ODR_RUNTIME)) {
			struct sensor_value odr = {
				.val1 = 1,
			};

			rc = sensor_attr_set(sensor, trig.chan,
					     SENSOR_ATTR_SAMPLING_FREQUENCY,
					     &odr);
			if (rc != 0) {
				printf("Failed to set odr: %d\n", rc);
				return;
			}
			printf("Sampling at %u Hz\n", odr.val1);
		}

		rc = sensor_trigger_set(sensor, &trig, trigger_handler);
		if (rc != 0) {
			printf("Failed to set trigger: %d\n", rc);
			return;
		}

		printf("Waiting for triggers\n");
		while (true) {
			k_sleep(K_MSEC(2000));
		}
	}
#else /* CONFIG_LIS2DH_TRIGGER */
	printf("Polling at 0.5 Hz\n");
	while (true) {
		fetch_and_display(sensor);
		k_sleep(K_MSEC(50));
	}
#endif /* CONFIG_LIS2DH_TRIGGER */
}




