/*
 * ==================================================================
 * Simple Keypad
 * rev 1 - October 2026 - shabaz
 *
 * Hardware Pinout:
 *   - GPIO7   : green - SER_DAT (HC595 pin 14)
 *   - GPIO8   : brown - SR_CLK (HC595 pin 11)
 *   - GPIO9   : purple - SW_DATA (switch/diode common)
 *   - GPIO10  : blue - *SR_CLR (HC595 pin 10)
 *   - GPIO11  : white - R_CLK (HC595 pin 12)
 *   - GPIO25  : Onboard status LED (RP2040 Pico)
 * ==================================================================
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"



// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// GPIO
#define SER_DAT_GPIO  7
#define SR_CLK_GPIO   8
#define SW_DATA_GPIO  9
#define SR_CLR_GPIO   10
#define R_CLK_GPIO    11
#define LED_GPIO      25

// Hardware config
#define INPUT_BLOCKS     2
#define MAP_KEY_BYTENUM 0
#define MAP_KEY_INDEX   1
#define OUTPUT_BLOCKS 1

// Helpers
#define NS32_DELAY {__asm volatile("nop"); __asm volatile("nop"); __asm volatile("nop"); __asm volatile("nop");}
#define SR_CLK_PULSE  {gpio_put(SR_CLK_GPIO, 1); NS32_DELAY; NS32_DELAY; gpio_put(SR_CLK_GPIO, 0);}
#define R_CLK_PULSE   {gpio_put(R_CLK_GPIO, 1);  NS32_DELAY; NS32_DELAY; gpio_put(R_CLK_GPIO, 0);}

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
void board_init(void);
void clear_shift_registers(void);
uint8_t io_scan_once(uint8_t* btn_arr, uint8_t* out_arr);


// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
void board_init(void) {
    gpio_init(LED_GPIO); gpio_set_dir(LED_GPIO, GPIO_OUT);
    gpio_put(LED_GPIO, 0);

    stdio_init_all();

    gpio_init(SR_CLR_GPIO); gpio_set_dir(SR_CLR_GPIO, GPIO_OUT);
    gpio_put(SR_CLR_GPIO, 0);
    gpio_init(SER_DAT_GPIO); gpio_set_dir(SER_DAT_GPIO, GPIO_OUT);
    gpio_init(SR_CLK_GPIO); gpio_set_dir(SR_CLK_GPIO, GPIO_OUT);
    gpio_init(SW_DATA_GPIO); gpio_set_dir(SW_DATA_GPIO, GPIO_IN);
    // gpio_pull_down(SW_DATA_GPIO); // uncomment if there's no pull-down resistor
    gpio_init(R_CLK_GPIO); gpio_set_dir(R_CLK_GPIO, GPIO_OUT);

    sleep_ms(100); // let any attached hardware settle

    clear_shift_registers();
}

void clear_shift_registers(void) {
    gpio_put(SR_CLR_GPIO, 0);
    gpio_put(SR_CLR_GPIO, 1);
    R_CLK_PULSE;
}

uint8_t io_scan_once(uint8_t* btn_arr, uint8_t* out_arr) {
    int i, j;
    uint8_t bitmask;
    gpio_put(SER_DAT_GPIO, 0);
    gpio_put(SR_CLK_GPIO, 0);
    gpio_put(R_CLK_GPIO, 0);
    // button input scan
    for (j=0; j<INPUT_BLOCKS; j++) {
        *btn_arr = 0;
        for (i=0; i<8; i++) {
            if (j==0 && i==0) {
                gpio_put(SER_DAT_GPIO, 1);
                SR_CLK_PULSE;
                gpio_put(SER_DAT_GPIO, 0);
            } else {
                SR_CLK_PULSE;
            }
            R_CLK_PULSE;
            NS32_DELAY; NS32_DELAY; NS32_DELAY; NS32_DELAY;
            NS32_DELAY; NS32_DELAY; NS32_DELAY; NS32_DELAY;
            if (gpio_get(SW_DATA_GPIO)) {
                *btn_arr = (*btn_arr>>1) | 0x80;
            } else {
                *btn_arr = (*btn_arr>>1);
            }
        }
        btn_arr++;
    }

    // set any outputs and push through past the input registers
    for (j=0; j<OUTPUT_BLOCKS; j++) {
        bitmask = 0x80;
        for (i=0; i<8; i++) {
            if (*out_arr & bitmask) {
                gpio_put(SER_DAT_GPIO, 1);
            } else {
                gpio_put(SER_DAT_GPIO, 0);
            }
            //NS32_DELAY; NS32_DELAY; NS32_DELAY; NS32_DELAY;
            SR_CLK_PULSE;
            bitmask = bitmask >> 1;
        }
    }
    gpio_put(SER_DAT_GPIO, 0);
    for (j=0; j<INPUT_BLOCKS; j++) {
        for (i=0; i<8; i++) {
            SR_CLK_PULSE;
        }
    }
    //NS32_DELAY; NS32_DELAY; NS32_DELAY; NS32_DELAY;
    R_CLK_PULSE;

    return(0);
}


// -----------------------------------------------------------------------------
// main function
// -----------------------------------------------------------------------------
int main(void) {
    uint8_t btn_arr[INPUT_BLOCKS];
    uint8_t leds_pattern = 0;
    uint8_t led_pattern_time = 0;


    board_init();
    gpio_put(LED_GPIO, 0);

    while (true) {
        if (led_pattern_time == 5) {
            led_pattern_time = 0;
            leds_pattern++;
            if (leds_pattern == 8) {
                leds_pattern = 0;
            }
        }
        //leds_pattern = 0x07; // test
        io_scan_once(btn_arr, &leds_pattern);
        printf("val: 0x%02x 0x%02x\n", btn_arr[1], btn_arr[0]);
        sleep_ms(100);
        led_pattern_time++;
        tight_loop_contents();
    }
}
