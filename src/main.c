/**
 * @file main.c
 * @author Trevor Heyl trevor@planet3.co.za
 * @brief 
 * @version 0.1
 * @date 2022-06-20
 * 
 * MIT license
 * 
 * Sample Zephyr application using timer and work queues but not threads
 * A Timer callback queues up messages for a foregound loop to consume
 */

#include <zephyr.h>
#include <drivers/gpio.h>


#define LEDRED_NODE DT_ALIAS(ledred)
#define LEDGREEN_NODE DT_ALIAS(ledgreen)
#define LEDBLUE_NODE DT_ALIAS(ledblue)


static const struct gpio_dt_spec ledred   = GPIO_DT_SPEC_GET(LEDRED_NODE, gpios);
static const struct gpio_dt_spec ledgreen = GPIO_DT_SPEC_GET(LEDGREEN_NODE, gpios);
static const struct gpio_dt_spec ledblue  = GPIO_DT_SPEC_GET(LEDBLUE_NODE, gpios);


void pf_green_callback(struct k_timer *timer_id);
void pf_red_callback(struct k_timer *timer_id);
void red_led_work_handler(struct k_work *work);


/**
 * @brief Define Timer
 *  Timers can be defined at compile time or run time, the macro version defines it at compile time
 *  Alternately we can define the timer a follows:
 * struct k_timer green_timer;
 * k_timer_init(&green_timer, pf_green_callback, NULL);
 * The macro below defines a timer with 
 *  - variable name  : green_timer
 *  - timer callback function : pf_green_callabck
 *  - timer stop callback function : NULL (no function)
 * 
 */
K_TIMER_DEFINE(green_timer, pf_green_callback, NULL); /* NULL here is no function for timer stop */

#define TIMER_GREEN_INITIAL_TO  K_SECONDS(6)
#define TIMER_GREEN_REPEAT_TO 	K_SECONDS(1)
#define TIMER_RED_INITIAL_TO  	K_SECONDS(1)
#define TIMER_RED_REPEAT_TO 	K_MSEC(200)

/**
 * @brief Construct a new k work define object
 *  The macro below defines a work queue with:
 *  - variable name : red_led_work
 *  - The handler function whwn the queue has work : red_led_work_handler
 */
K_WORK_DEFINE(red_led_work, red_led_work_handler);

/**
 * @brief  The callback function for the green led timer
 * 
 * @param timer_id 
 */
void pf_green_callback(struct k_timer *timer_id){

	gpio_pin_toggle_dt(&ledgreen); /* Bad exmaple, dont too processing in interrupt, \
										a better way to do work in a timer callback is to use work queues - eliminates blocking */
}

/**
 * @brief The callback function for the red led timer
 * 
 */
void pf_red_callback(struct k_timer *timer_id){

	k_work_submit(&red_led_work); /* signal a queue that there is work to do,this puts a message in the red_led_work queue */
}


void red_led_work_handler(struct k_work *work)
{
    /* do the processing that needs to be done periodically, triggered by the red led timer */
   	gpio_pin_toggle_dt(&ledred);
}



/**
* init_failed()
* function called if there is any kind of zephyr initialisation problem
*/
void init_failed(void) {

	printk("Zephyr Init failed\n");
	while(1);  // Just a simple trap for errors
}



/**
* main()
* main -> check for proper zephyr inits then just toggle all 3 leds in a loop
*/
void main(void)
{
	int ret;
	printk("Hello World! %s\n", CONFIG_BOARD);

      /* Check if all devices initalised properly */
	if (!device_is_ready(ledred.port)) {
		init_failed();
	}
	if (!device_is_ready(ledgreen.port)) {
		init_failed();
	}
	if (!device_is_ready(ledblue.port)) {
		init_failed();
	}

	/* make all led pins output */
	ret = gpio_pin_configure_dt(&ledred, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		init_failed();
	}

	ret = gpio_pin_configure_dt(&ledgreen, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		init_failed();
	}

	ret = gpio_pin_configure_dt(&ledblue, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		init_failed();
	}

	/* This timer is defined when main runs */
	struct k_timer red_timer;
 	k_timer_init(&red_timer, pf_red_callback, NULL);
 
	/* Start timers: timer init is need to know which timer object to init and the initial timeout and the repeat timeout */
	k_timer_start(&green_timer, TIMER_GREEN_INITIAL_TO, TIMER_GREEN_REPEAT_TO);
	k_timer_start(&red_timer, TIMER_RED_INITIAL_TO, TIMER_RED_REPEAT_TO);

	/***** Main loop  ******/
	/* There is green and Rred led flashing happening in the background */
	while(1) {
		k_msleep(500);
		printk("+");
	
	}
}
