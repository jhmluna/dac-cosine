/* Timer Functions header files
 * Contains variables and functions related to the timer.
* It uses the timer interrupt to periodically enable and disable the AD output
*/

#ifndef TIMER_FUNCTIONS_H
#define TIMER_FUNCTIONS_H

#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/timer.h"

#define TIMER_DIVIDER 16                                      //  Hardware timer clock divider
#define TIMER_SCALE ((TIMER_BASE_CLK / TIMER_DIVIDER) / 1000) // convert counter value to miliseconds
#define TIMER_INTERVAL0_SEC (3000)                            // Disable interval for the first timer in miliseconds
#define TIMER_INTERVAL1_SEC (100)                            // Enable interval for the second timer in miliseconds
#define TEST_WITHOUT_RELOAD 0                                 // testing will be done without auto reload
#define TEST_WITH_RELOAD 1                                    // testing will be done with auto reload

/* A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct
{
    bool timer0IntFlag;
    bool timer1IntFlag;
} timer_event_t;

/* Queue to transfer data between timer_group0_isr and timer_example_evt_task */
xQueueHandle timer_queue;

/* Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para);

/* Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_msec - the interval of alarm to set
 */
void example_tg0_timer_init(int timer_idx,
                            bool auto_reload, double timer_interval_msec);

#endif