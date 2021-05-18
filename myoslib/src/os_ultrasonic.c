/**
 **********************************************************************
 * @file        os_ultrasonic.c
 * @author      Tunyun He - s4466833
 * @date        19/3/2021
 * @brief       ultrasonic 
 *
 **************************************************************************
 */



#include "os_ultrasonic.h"

unsigned int distance = 0;
unsigned long pulse_width = 0;
const struct device *dev_trigger;
const struct device *dev_echo;
bool high = true; // tirgger flag
int ret_echo;
int ret_trigger;

extern unsigned long ultrasonic_sensor(void) {

	// Loop count calculate time duration
	unsigned long loopCount = 0;

	// Reset pulse with
	pulse_width = 0;

	// Set the maximum measured distance
	unsigned long loopMax = 25000;


        gpio_pin_set(dev_trigger, PIN_TRIGGER, (int)!high);// set trigger pin to low
        k_usleep(GPIO_SLEEP_TIME_NS); //wait for 2us 
        
	gpio_pin_set(dev_trigger, PIN_TRIGGER, (int)high);// set trigger pin to high
	k_usleep(SLEEP_TIME_NS); //wait for 10us
	
	gpio_pin_set(dev_trigger, PIN_TRIGGER, 0);// set trigger pin to high and the trigger condition has met

	// While the pin is not in the trigger state, clear the timeout flag.
    while ((unsigned int) gpio_pin_get(dev_echo, PIN_ECHO) == 0) {
        if (loopCount++ == loopMax) {
            return loopMax;
        }
    }	
	
    // When the pin is in the trigger state, start the counter while keeping track of the timeout.
    while ((unsigned int)gpio_pin_get(dev_echo, PIN_ECHO) == 1) {
        if (loopCount++ == loopMax) {
            return loopMax;
        }
        pulse_width++;
    }
    
    // Return the pulse time in microsecond!
    return (pulse_width); // Calculated the pulseWidth++ loop to be about 2.36uS in length.
}

extern void ultrasonic_init(void){

    dev_trigger = device_get_binding(D0);// enable port for trigger pin
	dev_echo = device_get_binding(D1);// enable port for echo pin
	if (dev_trigger == NULL) {
		return;
	}
	
	if (dev_echo == NULL) {
		return;
	}

	ret_trigger = gpio_pin_configure(dev_trigger, PIN_TRIGGER, GPIO_OUTPUT_ACTIVE | FLAGS_TRIGGER);// configure trigger pin as output 
	if (ret_trigger < 0) {
		return;
	}
	
	ret_echo = gpio_pin_configure(dev_echo, PIN_ECHO, GPIO_INPUT | FLAGS_ECHO);// configure echo pin as input
	if (ret_echo < 0) {
		return;
	}
	
	gpio_pin_set(dev_trigger, PIN_TRIGGER, 0);
	

}


