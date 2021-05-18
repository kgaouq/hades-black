/**
***************************************************************
* @file     apps/p1/src/main.c
* @author   Kevin Gao - 4436802
* @date     16032021
* @brief    main file for prac 1
*       REFERENCE: csse4011_prac1.pdf
***************************************************************
***
*/

#include "main.h"
/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>

static void fetch_and_display(const struct shell *shell, const struct device *sensor)
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
		shell_print(shell, "ERROR: Update failed: %d\n", rc);
	} else {
		shell_print(shell, "#%u @ %u ms: %sx %f , y %f , z %f\n",
		       count, k_uptime_get_32(), overrun,
		       sensor_value_to_double(&accel[0]),
		       sensor_value_to_double(&accel[1]),
		       sensor_value_to_double(&accel[2]));
	}
}

#ifdef CONFIG_LIS2DH_TRIGGER
static void trigger_handler(const struct device *dev,
			    struct sensor_trigger *trig)
{
	//fetch_and_display(dev);
}
#endif
const struct device *sensor;

void cmd_acc(const struct shell *shell, size_t argc, char **argv)
{

	if (sensor == NULL) {
		shell_print(shell, "Could not get %s device\n",
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
				shell_print(shell, "Failed to set odr: %d\n", rc);
				return;
			}
			shell_print(shell, "Sampling at %u Hz\n", odr.val1);
		}

		rc = sensor_trigger_set(sensor, &trig, trigger_handler);
		if (rc != 0) {
			shell_print(shell, "Failed to set trigger: %d\n", rc);
			return;
		}

		shell_print(shell, "Waiting for triggers\n");
		while (true) {
			k_sleep(K_MSEC(2000));
		}
	}
#else /* CONFIG_LIS2DH_TRIGGER */
	shell_print(shell, "Polling at 0.5 Hz\n");
	while (true) {
		fetch_and_display(shell, sensor);
		k_sleep(K_MSEC(100));
	}
#endif /* CONFIG_LIS2DH_TRIGGER */
}

void main(void){
 sensor = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));
}

/* Create shell command time */
SHELL_CMD_REGISTER(acc, NULL, "Print time commands", cmd_acc);
