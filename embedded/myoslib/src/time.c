/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
***************************************************************
* @file     myoslib/src/time.c
* @author   Kevin Gao - 4436802
* @date     16032021
* @brief    Kernel time Shell Command
*       REFERENCE: csse4011_prac1.pdf
***************************************************************
*      EXTERNAL FUNCTIONS
***************************************************************
*
***************************************************************
*/

#include "time.h"

/* Shell command to display time in second*/
static int cmd_time_s(const struct shell *shell, size_t argc, char **argv) {
    int64_t millisecond = k_uptime_get();
    int64_t second = millisecond / 1000;
	shell_print(shell, "%lld", second);
	return 0;
}

/* Shell command to display time in HH:MM:SS format*/
static int cmd_time_f(const struct shell *shell, size_t argc, char **argv) {
    int64_t millisecond = k_uptime_get();
    int64_t second = millisecond / 1000 % 60;
    int64_t min = millisecond / 60000 % 60;
    int64_t hour = millisecond / 3600000;
	shell_print(shell, "%02lld:%02lld:%02lld", hour, min, second);
	return 0;
}

/* Create subcommand for time */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_time,
    SHELL_CMD(f, NULL, "Print in format time.", cmd_time_f),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

/* Create shell command time */
SHELL_CMD_REGISTER(time, &sub_time, "Print time commands", cmd_time_s);

