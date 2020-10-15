#include "timer_functions.h"

/* Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
    timer_spinlock_take(TIMER_GROUP_0);
    timer_event_t evt;
    evt.timer0IntFlag = pdFALSE;
    evt.timer1IntFlag = pdFALSE;

    /* Retrieve the interrupt status
       from the timer that reported the interrupt */
    uint32_t timer_intr = timer_group_get_intr_status_in_isr(TIMER_GROUP_0);

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if (timer_intr & TIMER_INTR_T0)
    {
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
        evt.timer0IntFlag = pdTRUE;
        /* After the alarm has been triggered
         * we need enable it again, so it is triggered the next time
        */
        timer_pause(TIMER_GROUP_0, TIMER_0);
        timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0x00000000ULL);
        timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_1);
        timer_start(TIMER_GROUP_0, TIMER_1);
    }
    else if (timer_intr & TIMER_INTR_T1)
    {
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
        evt.timer1IntFlag = pdTRUE;
        /* After the alarm has been triggered
         * we need enable it again, so it is triggered the next time
        */
        timer_pause(TIMER_GROUP_0, TIMER_1);
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
        timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
        timer_start(TIMER_GROUP_0, TIMER_0);
    }

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
    timer_spinlock_give(TIMER_GROUP_0);
}

/* Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_msec - the interval of alarm to set
 */
void example_tg0_timer_init(int timer_idx,
                                   bool auto_reload, double timer_interval_msec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_msec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
                       (void *)timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}