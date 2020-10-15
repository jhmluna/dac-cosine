/* Tone Genrator
 * It uses the functionality of ESP32's cosine waveform generator and develop a simple API to operate it.
 * It also uses UART0 to read integers from 0 to 65536 from the terminal to update the frequency step and,
 * consequently, update the generated frequency.
 * Lastly, timer0 interrupt enable and disable periodically the DAC outuput.
 * This sotware was based in 2 examples:
 * 1 - DAC Cosine Generator Example: This example was created from the source: https://github.com/krzychb/dac-cosine
 * 2 - Timer group-hardware timer example: Source: https://github.com/espressif/esp-idf/tree/8bc19ba893e5544d571a753d82b44a84799b94b1/examples/peripherals/timer_group
 */

#include <string.h>
#include "timer_functions.h"
#include "dac_functions.h"

/* DAC task that let you test CW parameters in action
 *
*/
void vDacTask(void *arg)
{
    while (1)
    {

        // frequency setting is common to both channels
        dac_frequency_set(clk_8m_div, frequency_step);

        // Tune parameters of channel 2 only to see and compare changes against channel 1
        dac_scale_set(DAC_CHANNEL_2, scale);
        dac_offset_set(DAC_CHANNEL_2, offset);
        dac_invert_set(DAC_CHANNEL_2, invert);

        // float frequency = RTC_FAST_CLK_FREQ_APPROX / (1 + clk_8m_div) * (float)frequency_step / 65536;
        //printf("clk_8m_div: %d, frequency step: %d, frequency: %.0f Hz\n", clk_8m_div, frequency_step, frequency);
        //printf("DAC2 scale: %d, offset %d, invert: %d\n", scale, offset, invert);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

/* UART task to read the input parameter and update the interval between pulses.
 * Configure UART, install the driver and make the parameterization.
 * Enter an infinite loop to check if data is available on the UART.
 * It would be better if it was done with interruption.
*/
static void vUartTask(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // Install UART driver
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    // Configure UART parameters
    uart_param_config(uart_num, &uart_config);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    // Boolean flag to direct the uart data to a specific variable to be changed.
    bool changeFrequency = pdFALSE;

    while (1)
    {
        // Check if there are data from UART.
        int length = 0, uartData = 0;
        uart_get_buffered_data_len(uart_num, (size_t *)&length);
        // If there are incoming data, treat them and update the interval between pulses..
        if (length > 0)
        {
            uart_read_bytes(uart_num, data, length, 100);
            if (data[0] == '!')
            {
                // If you receive "!", It switches between changing the frequency and the interval between pulses.
                changeFrequency = pdTRUE - changeFrequency;
                if (changeFrequency)
                {
                    printf("Frequency change\n");
                }
                else
                {
                    printf("Time invterval change\n");
                }
            }
            else
            {
                uartData = atoi((const char *)data);
                if ((uartData < 0) || (uartData > 65536))
                {
                    printf("Input number is invalid.\n");
                }
                else
                {
                    if (changeFrequency)
                    {
                        /* Change frequency */
                        frequency_step = uartData;
                        float frequency = RTC_FAST_CLK_FREQ_APPROX / (1 + clk_8m_div) * (float)frequency_step / 65536;
                        printf("New frequency: %5.2f Hz\n", frequency);
                    }
                    else
                    {
                        /* Change interval between pulses */
                        printf("New Interval: %d ms\n", uartData);
                        example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD, uartData);
                    }
                }
                uart_flush(uart_num);
                bzero(data, BUF_SIZE);
            }
        }
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
}

/* The main task of this example program
 */
static void timer_example_evt_task(void *arg)
{
    while (1)
    {
        timer_event_t evt;
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        /* Print information that the timer reported an event */
        if (evt.timer0IntFlag == pdTRUE)
        {
            dac_output_enable(DAC_CHANNEL_2);
            printf("\n    Enable DAC output - timer 1\n");
        }
        else if (evt.timer1IntFlag == pdTRUE)
        {
            dac_output_disable(DAC_CHANNEL_2);
            printf("\n    Disable DAC output - timer 0\n");
        }
    }
}

void app_main(void)
{
    dac_cosine_enable(DAC_CHANNEL_2);
    dac_output_enable(DAC_CHANNEL_2);

    timer_queue = xQueueCreate(2, sizeof(timer_event_t));
    example_tg0_timer_init(TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_SEC);
    example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD, TIMER_INTERVAL1_SEC);
    xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
    xTaskCreate(vDacTask, "dac_task", 1024 * 3, NULL, 1, NULL);
    xTaskCreate(vUartTask, "uart_task", 1024 * 2, NULL, 2, NULL);
}