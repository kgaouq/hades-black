/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef  LED_H
#define  LED_H

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <shell/shell.h>
#include <version.h>
#include <logging/log.h>
#include <stdlib.h>

/* define */
#define LED3_NODE DT_ALIAS(led3)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define LED3	DT_GPIO_LABEL(LED3_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED3_NODE, gpios)
#define FLAGS0	DT_GPIO_FLAGS(LED3_NODE, gpios)

#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)

#define LED2	DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN2	DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS2	DT_GPIO_FLAGS(LED2_NODE, gpios)


#define ON 0
#define OFF 1
#define TOGGLE 3

/* External functions*/
extern void led_init(void);

#endif


