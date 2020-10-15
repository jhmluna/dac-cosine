/* DAC Functions header files
 * Contains variables and functions related to the AD converter.
 * It uses the functionality of ESP32's cosine waveform generator and develop a simple API to operate it.
 */

#ifndef DAC_FUNCTIONS_H
#define DAC_FUNCTIONS_H

#include "driver/uart.h"
#include "driver/dac.h"
#include "soc/sens_reg.h"
#include "soc/rtc.h"


/* Declare global sine waveform parameters
 * so they may be then accessed and changed from debugger
 * over an JTAG interface
 */
int clk_8m_div;     // RTC 8M clock divider (division is by clk_8m_div+1, i.e. 0 means 8MHz frequency)
int frequency_step; // Frequency step for CW generator
int scale;          // 50% of the full scale
int offset;         // leave it default / 0 = no any offset
int invert;         // invert MSB to get sine waveform

/* Declare global uart parameters
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */
#define BUF_SIZE (128)
const uart_port_t uart_num;

/* Enable cosine waveform generator on a DAC channel
*/
void dac_cosine_enable(dac_channel_t channel);

/* Set frequency of internal CW generator common to both DAC channels
 *
 * clk_8m_div: 0b000 - 0b111
 * frequency_step: range 0x0001 - 0xFFFF
 *
 */
void dac_frequency_set(int clk_8m_div, int frequency_step);

/* Scale output of a DAC channel using two bit pattern:
 *
 * - 00: no scale
 * - 01: scale to 1/2
 * - 10: scale to 1/4
 * - 11: scale to 1/8
 *
 */
void dac_scale_set(dac_channel_t channel, int scale);

/* Offset output of a DAC channel
 *
 * Range 0x00 - 0xFF
 *
 */
void dac_offset_set(dac_channel_t channel, int offset);

/* Invert output pattern of a DAC channel
 *
 * - 00: does not invert any bits,
 * - 01: inverts all bits,
 * - 10: inverts MSB,
 * - 11: inverts all bits except for MSB
 *
 */
void dac_invert_set(dac_channel_t channel, int invert);

#endif