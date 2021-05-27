/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
***************************************************************
* @file     myoslib/src/hal_hci.c
* @author   Kevin Gao - 4436802
* @date     13042021
* @brief    SPI HCI Protocol Shell Command
*       REFERENCE: csse4011_prac2.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
* hal_init() - intialise hal hci and shell
***************************************************************
*/

#include "hal_hci.h"

/* private variables */
const struct device *spi;
struct spi_config spi_cfg;
const struct spi_cs_control ctrl = {
    .gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
    .delay = 0,
    .gpio_pin = 0x1f,
    .gpio_dt_flags = GPIO_ACTIVE_LOW};
uint8_t sensors[] = {LSM6DSL, LIS3MDL, LPS22HB, VL53L0X, HTS221};

extern void hci_send(uint8_t* data){
    uint8_t tx[2];
    tx[0] = PREAMBLE;
    tx[1] =  REQUEST << 4 | sizeof(data);
    const struct spi_buf tx_bufs[] = {
        {
            .buf = (uint8_t *) tx,
            .len = 2
        },
        {
            .buf = data,
            .len = sizeof(data)
        }
    };
    
    const struct spi_buf_set tx_set = {
        .buffers = tx_bufs,
        .count = ARRAY_SIZE(tx_bufs)
    };
    spi_write(spi, &spi_cfg, &tx_set);
}

extern void hci_get(uint8_t* data){
    uint8_t pre[2] = {0};
    const struct spi_buf rx_bufs[] = {
        {
            .buf = pre,
            .len = 2
        },
        {
            .buf = data,
            .len = 4
        }
    };
    
    const struct spi_buf_set rx_set = {
        .buffers = rx_bufs,
        .count = 2
    };
    int counter = 0;
    while((pre[0] != PREAMBLE || !(pre[1] & RESPONSE << 4)) && counter < 50){
        spi_read(spi, &spi_cfg, &rx_set);
        counter++;
        k_msleep(10);
    }

}

extern int get_ultra(uint8_t* rx_data){
    uint8_t SID = 0x06;
    uint8_t data[] = {SID, 0x00, 0x00};
    hci_send((uint8_t*) data);
    hci_get(rx_data);
    return 0;
}

/* Set up SPI*/
extern void spi_hci_init(void) {
    spi = device_get_binding(SPI_DEV);
	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;
    spi_cfg.frequency = 4000000;
    spi_cfg.cs = &ctrl;
    spi_cfg.slave = 0;;
}

/* Shell command to read I2C sensor register*/
static int cmd_i2creg_r(const struct shell *shell, size_t argc, char **argv) {
    uint8_t SID = (uint8_t) atoi(argv[1]);
    uint8_t i2cAddr = sensors[atoi(argv[1]) - 1] << 1 | 0x01;
    uint8_t regaddr = (uint8_t) strtol(argv[2], NULL, 16);
    uint8_t data[] = {SID, i2cAddr, regaddr};
    hci_send((uint8_t*) data);
	return 0;
}

/* Shell command to write I2C sensor register*/
static int cmd_i2creg_w(const struct shell *shell, size_t argc, char **argv) {
    uint8_t SID = (uint8_t) atoi(argv[1]);
    uint8_t i2cAddr = sensors[atoi(argv[1]) - 1] << 1 & ~0x01;
    uint8_t regaddr = (uint8_t) strtol(argv[2], NULL, 16);
    uint8_t regval = (uint8_t) strtol(argv[3], NULL, 16);
    uint8_t data[] = {SID, i2cAddr, regaddr, regval};
    hci_send((uint8_t*) data);
	return 0;
}

static int cmd_ultra(const struct shell *shell, size_t argc, char **argv) {
    uint8_t rx_data[4] = {0};
    uint8_t SID = 0x06;
    uint8_t i2cAddr = 0x01;
    uint8_t regaddr = 0x00;
    uint8_t data[] = {SID, i2cAddr, regaddr};
    while(1){
        hci_send((uint8_t*) data);
        hci_get((uint8_t*) rx_data);
        shell_print(shell, "%d", rx_data[0]);
        k_msleep(100);
    }
	return 0;
}

/* Create subcommand for i2creg */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_i2creg,
	SHELL_CMD_ARG(r, NULL, "read I2C sensor register", cmd_i2creg_r, 3, 0),
	SHELL_CMD_ARG(w, NULL, "write I2C sensor register", cmd_i2creg_w, 4, 0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create shell command i2creg */
SHELL_CMD_REGISTER(i2creg, &sub_i2creg, "I2C sensor commands", NULL);

/* Create shell command LSM6DSL */
SHELL_CMD_REGISTER(lsm6dsl, NULL, "LSM6DSL commands", NULL);

/* Create shell command ultrasonic */
SHELL_CMD_REGISTER(ultra, NULL, "ultrasonic", cmd_ultra);



