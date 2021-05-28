/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>
#include <drivers/sensor.h>

#define CONN_NUM 3

struct ble_data{
    int8_t dev;
    int8_t indic;
    int32_t data;
};

struct sensor_value acc[3];

static struct bt_uuid_16 vnd_enc_uuid = BT_UUID_INIT_16(0xf1de);

static void start_scan(void);

static struct bt_conn *default_conn;
static struct bt_conn *conns[CONN_NUM];

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params[CONN_NUM];
static struct bt_gatt_subscribe_params subscribe_params[CONN_NUM];

int get_conn_index(struct bt_conn *conn){
    int i;
    for(i = 0; i < CONN_NUM; i++){
        if(conn == conns[i]){
            return i;
        }
    } 
    return -1;
}

int get_empty_conn(void){
    int i;
    for(i = 0; i < CONN_NUM; i++){
        if(conns[i] == NULL){
            return i;
        }
    } 
    return -1;
}

static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (!data) {
		printk("[UNSUBSCRIBED]\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}
    struct ble_data *ble = (struct ble_data*) data;
    /* json print format goes here */
    int i = (ble->indic)/2;
    int j = (ble->indic)%2;
    if(j){
        acc[i].val2 = ble->data;
    }else{
        acc[i].val1 = ble->data;
    } 
	
	return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params)
{
    int index = get_conn_index(conn);
	int err;

	if (!attr) {
		printk("Discover complete\n");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	printk("[ATTRIBUTE] handle %u\n", attr->handle);

	if (!bt_uuid_cmp(discover_params[index].uuid, &vnd_enc_uuid.uuid)) {
		memcpy(&uuid, BT_UUID_GATT_CCC, sizeof(uuid));
		discover_params[index].uuid = &uuid.uuid;
		discover_params[index].start_handle = attr->handle + 2;
		discover_params[index].type = BT_GATT_DISCOVER_DESCRIPTOR;
		subscribe_params[index].value_handle = bt_gatt_attr_value_handle(attr);

		err = bt_gatt_discover(conn, &(discover_params[index]));
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}
	} else {
		subscribe_params[index].notify = notify_func;
		subscribe_params[index].value = BT_GATT_CCC_NOTIFY;
		subscribe_params[index].ccc_handle = attr->handle;

		err = bt_gatt_subscribe(conns[index], &(subscribe_params[index]));
		if (err && err != -EALREADY) {
			printk("Subscribe failed (err %d)\n", err);
		} else {
			printk("[SUBSCRIBED]\n");
		}

		return BT_GATT_ITER_STOP;
	}
	return BT_GATT_ITER_STOP;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char dev[BT_ADDR_LE_STR_LEN];
	int index = get_empty_conn();
	if(index == -1){
	    return;
	}

	bt_addr_le_to_str(addr, dev, sizeof(dev));
	printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
	       dev, type, ad->len, rssi);

	/* We're only interested in connectable events */
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		struct bt_le_conn_param *param;
        param = BT_LE_CONN_PARAM_DEFAULT;
        int err;
        err = bt_le_scan_stop();
        if (err) {
		    printk("Stop LE scan failed (err %d)\n", err);
		    return;
	    }
	    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
						param, &(conns[index]));
	    if (err){
	        printk("Create conn failed (err %d)\n", err);
	        start_scan();
	    }
	}
}

static void start_scan(void)
{
	int err;

	/* Use active scanning and disable duplicate filtering to handle any
	 * devices that might update their advertising data at runtime. */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_FILTER_WHITELIST,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
    int index = get_conn_index(conn);
    
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		printk("Failed to connect to %s (%u)\n", addr, conn_err);

		bt_conn_unref(conns[index]);
		conns[index] = NULL;

		start_scan();
		return;
	}

	printk("Connected: %s\n", addr);

	if (index != -1) {
		discover_params[index].uuid = &vnd_enc_uuid.uuid;
		discover_params[index].func = discover_func;
		discover_params[index].start_handle = 0x0001;
		discover_params[index].end_handle = 0xffff;
		discover_params[index].type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conns[index], &(discover_params[index]));
		if (err) {
			printk("Discover failed(err %d)\n", err);
			start_scan();
			return;
		}
	}
	start_scan();
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    int index = get_conn_index(conn);
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	if (index == -1) {
		return;
	}

	bt_conn_unref(conns[index]);
	conns[index] = NULL;

	start_scan();
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

void console_init(void){
	const struct device *dev = device_get_binding(
			CONFIG_UART_CONSOLE_ON_DEV_NAME);
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return;
	}

	/* Poll if the DTR flag was set, optional */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	}

	if (strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME) !=
	    strlen("CDC_ACM_0") ||
	    strncmp(CONFIG_UART_CONSOLE_ON_DEV_NAME, "CDC_ACM_0",
		    strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME))) {
		printk("Error: Console device name is not USB ACM\n");

		return;
	}

}

void main(void)
{
    console_init();
	int err;
	err = bt_enable(NULL);
	
	bt_addr_le_t addr;
	const char *addr_str = "C8:91:07:19:03:58";
	const char *type = "random";
	bt_addr_le_from_str(addr_str, type, &addr);
	bt_le_whitelist_add(&addr);

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	bt_conn_cb_register(&conn_callbacks);

	start_scan();
	while(1){
	    sensor_value_to_double(&accel[0]);
	}
}
