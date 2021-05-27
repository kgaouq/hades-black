/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef  HAL_HCI_H
#define  HAL_HCI_H

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <shell/shell.h>
#include <version.h>
#include <logging/log.h>
#include <stdlib.h>
#include <errno.h>
#include <drivers/spi.h>
#include "hci_packet.h"

/* define */
#define SPI_DEV DT_LABEL(DT_PATH(soc, spi_40004000))

/* External functions*/
extern void spi_hci_init(void);
extern void hci_send(uint8_t* data);
extern void hci_get(uint8_t* data);
extern int get_ultra(uint8_t* rx_data);

#endif


