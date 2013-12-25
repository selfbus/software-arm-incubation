// main.c

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "gpio.h"
#include "uart.h"

#include "sb_main.h"
#include "sb_comobj.h"

#include "sb_bus.h"

#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led


int main()
{
    int sendTrigger = 0;
    unsigned char testValue = 1;

    sb_init();
    sb_set_appdata(0, 4, 0x7054, 2);  // we are a "Jung 2118" device, version 0.2

    UART_Init(115200);
    UART_PutString("in8-bcu1\n", -1);

    GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);


    while (1)
    {
        sb_main_loop();

        if (sbSendCurTelegram == 0 && sbState == SB_IDLE)
            __WFI();  // goto sleep until an interrupt happens

        // Send a specific telegram if P1.11 is low
        if (GPIOGetValue(1, 11) == 0)
        {
            if (sendTrigger < 10)
            {
                if (++sendTrigger == 9)
                {
                    GPIOSetValue(LED_PORT, LED_BIT, 1);

                    unsigned char* valuePtr = sb_get_value_ptr(0);
                    *valuePtr = testValue;
                    testValue ^= 1;

                    sb_send_obj_value(0);
                }
            }
        }
        else if (sendTrigger > 0)
        {
            if (!--sendTrigger)
                GPIOSetValue(LED_PORT, LED_BIT, 0);
        }
    }

    return 0;
}
