/**
 **********************************************************************
 * @file        os_shell.c
 * @author      Tunyun He - s4466833
 * @date        19/3/2021
 * @brief       shell applications 
 *
 **************************************************************************
 */



#ifndef OS_ULTRASONIC_H
#define OS_ULTRASONIC_H
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_NS  6.2
#define GPIO_SLEEP_TIME_NS  97.5

/* The devicetree node identifier for the "led0" alias. */

#define D0	"GPIOA"
#define PIN_TRIGGER	1
#define FLAGS_TRIGGER	0

#define D1	"GPIOA"
#define PIN_ECHO	0
#define FLAGS_ECHO	0
/* A build error here means your board isn't set up to blink an LED. */



extern unsigned long ultrasonic_sensor(void);

extern void ultrasonic_init(void);

#endif


