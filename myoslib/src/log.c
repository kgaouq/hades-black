/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
***************************************************************
* @file     myoslib/src/log.c
* @author   Kevin Gao - 4436802
* @date     16032021
* @brief    Log Shell Command
*       REFERENCE: csse4011_prac1.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
*
***************************************************************
*/

#include "log.h"

/* Create subcommand for log */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_time,
    SHELL_CMD(e, NULL, "Print in format time.", NULL),
    SHELL_CMD(d, NULL, "Print in format time.", NULL),
    SHELL_CMD(l, NULL, "Print in format time.", NULL),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create shell command time */
SHELL_CMD_REGISTER(log, &sub_log, "Print time commands", NULL);

