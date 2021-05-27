/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
***************************************************************
* @file     myoslib/src/led.c
* @author   Kevin Gao - 4436802
* @date     16032021
* @brief    LED Shell Command
*       REFERENCE: csse4011_prac1.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* led_init() - intialise LED gpio
***************************************************************
*/

#include "led.h"

/* private variables */
const struct device *led_red;
const struct device *led_green;
const struct device *led_blue;
bool led_red_on;
bool led_blue_on;
bool led_green_on;

/* Set up LED */
extern void led_init(void) {
    int ret;
    bool led_red_on = false;
    bool led_blue_on = false;
    bool led_green_on = false;
    led_red = device_get_binding(LED1);
	if (led_red == NULL) {
		return;
	}
    led_green = device_get_binding(LED2);
	if (led_green == NULL) {
		return;
	}
    led_blue = device_get_binding(LED3);
	if (led_blue == NULL) {
		return;
	}
    ret = gpio_pin_configure(led_red, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure(led_green, PIN2, GPIO_OUTPUT_ACTIVE | FLAGS2);
	if (ret < 0) {
		return;
	}
	ret = gpio_pin_configure(led_blue, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0);
	if (ret < 0) {
		return;
	}

    gpio_pin_set(led_red, PIN1, (int)led_red_on);
    gpio_pin_set(led_green, PIN2, (int)led_green_on);
    gpio_pin_set(led_blue, PIN0, (int)led_blue_on);
}

/* Configuration for red LED */
void led_red_config(int mode) {
    switch(mode){
        case ON:
            led_red_on = true;
            gpio_pin_set(led_red, PIN1, (int)led_red_on);
            break;
        case OFF:
            led_red_on = false;
            gpio_pin_set(led_red, PIN1, (int)led_red_on);
            break;
        case TOGGLE:
            led_red_on = !led_red_on;
            gpio_pin_set(led_red, PIN1, (int)led_red_on);
    }
}

/* Configuration for green LED */
void led_green_config(int mode) {
    switch(mode) {
        case ON:
            led_green_on = true;
            gpio_pin_set(led_green, PIN2, (int)led_green_on);
            break;
        case OFF:
            led_green_on = false;
            gpio_pin_set(led_green, PIN2, (int)led_green_on);
            break;
        case TOGGLE:
            led_green_on = !led_green_on;
            gpio_pin_set(led_green, PIN2, (int)led_green_on);
    }
}

/* Configuration for blue LED */
void led_blue_config(int mode) {
    switch(mode) {
        case ON:
            led_blue_on = true;
            gpio_pin_set(led_blue, PIN0, (int)led_blue_on);
            break;
        case OFF:
            led_blue_on = false;
            gpio_pin_set(led_blue, PIN0, (int)led_blue_on);
            break;
        case TOGGLE:
            led_blue_on = !led_blue_on;
            gpio_pin_set(led_blue, PIN0, (int)led_blue_on);
    }
}

/* Shell command to turn red LED on*/
static int cmd_led_o_r(const struct shell *shell, size_t argc, char **argv) {
	led_red_config(ON);
	return 0;
}

/* Shell command to turn green LED on*/
static int cmd_led_o_g(const struct shell *shell, size_t argc, char **argv) {
	led_green_config(ON);
	return 0;
}

/* Shell command to turn blue LED on*/
static int cmd_led_o_b(const struct shell *shell, size_t argc, char **argv) {
	led_blue_config(ON);
	return 0;
}

/* Shell command to turn red LED off*/
static int cmd_led_f_r(const struct shell *shell, size_t argc, char **argv) {
	led_red_config(OFF);
	return 0;
}

/* Shell command to turn green LED off*/
static int cmd_led_f_g(const struct shell *shell, size_t argc, char **argv) {
	led_green_config(OFF);
	return 0;
}

/* Shell command to turn blue LED off*/
static int cmd_led_f_b(const struct shell *shell, size_t argc, char **argv) {
	led_blue_config(OFF);
	return 0;
}

/* Shell command to toggle red LED*/
static int cmd_led_t_r(const struct shell *shell, size_t argc, char **argv) {
	led_red_config(TOGGLE);
	return 0;
}

/* Shell command to toggle green LED*/
static int cmd_led_t_g(const struct shell *shell, size_t argc, char **argv) {
	led_green_config(TOGGLE);
	return 0;
}

/* Shell command to toggle blue LED */
static int cmd_led_t_b(const struct shell *shell, size_t argc, char **argv) {
	led_blue_config(TOGGLE);
	return 0;
}

/* Create subcommand for led o */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_o,
	SHELL_CMD(r, NULL, "Turn red LED on command.", cmd_led_o_r),
	SHELL_CMD(g, NULL, "Turn green LED on command.", cmd_led_o_g),
	SHELL_CMD(b, NULL, "Turn blue LED on command.", cmd_led_o_b),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create subcommand for led f */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_f,
	SHELL_CMD(r, NULL, "Turn red LED off command.", cmd_led_f_r),
	SHELL_CMD(g, NULL, "Turn green LED off command.", cmd_led_f_g),
	SHELL_CMD(b, NULL, "Turn blue LED off command.", cmd_led_f_b),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create subcommand for led t */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_t,
	SHELL_CMD(r, NULL, "Toggle red LED command.", cmd_led_t_r),
	SHELL_CMD(g, NULL, "Toggle green LED command.", cmd_led_t_g),
	SHELL_CMD(b, NULL, "Toggle blue LED command.", cmd_led_t_b),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create subcommand for led */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
	SHELL_CMD(o, &sub_o, "Turn LED on command.", NULL),
	SHELL_CMD(f, &sub_f, "Turn LED off command.", NULL),
	SHELL_CMD(t, &sub_t, "Toggle LED command.", NULL),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create shell command led */
SHELL_CMD_REGISTER(led, &sub_led, "LED commands", NULL);

