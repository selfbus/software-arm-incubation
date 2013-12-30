// main.c

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "gpio.h"
#include "uart.h"

#include "sb_bus.h"
#include "sb_proto.h"
#include "sb_main.h"
#include "sb_memory.h"

#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led

#ifdef EEP_TEST
extern void sb_eep_test (void);
#endif


#include "sb_timer.h"

void timer_test()
{
    int i;
    SbTimer timer;

    sb_timer_start(& timer, 0, 500000);
    for(i = 0;i < 10;i ++)
    {
        while(! sb_timer_check( &timer));
        GPIOSetValue(LED_PORT, LED_BIT, 1);
        while(! sb_timer_check( &timer));
        GPIOSetValue(LED_PORT, LED_BIT, 0);
    }
}

int main(void)
{
    int sendTrigger = 0;
    unsigned char testValue = 1;

    sb_init();
    timer_test();
    sb_set_appdata(0, 2, 0x9009, 0);

    UART_Init(115200);
    UART_PutString("ARM-SB-Lib Test\n", -1);

    // Set our own bus address (0x117e == 1.1.126)
    sbOwnPhysicalAddr = 0x117e;

#ifdef EEP_TEST
    sb_eep_test();
#endif

    GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);


    while (1)
    {
        sb_main_loop();

        // Send a specific telegram if P1.11 is low
        if (GPIOGetValue(1, 11) == 0)
        {
            if (sendTrigger < 10240 && sbSendCurTelegram == 0)
            {
                if (++sendTrigger == 10239)
                {
                    GPIOSetValue(LED_PORT, LED_BIT, 1);

                    sbSendTelegram[0] = 0xbc;
                    sbSendTelegram[1] = 0x0;  // 1+2: source address: will be set by sb_send_tel() to sbOwnPhysicalAddr
                    sbSendTelegram[2] = 0x0;
                    sbSendTelegram[3] = 0x11; // 3+4: dest address: 2/1/2
                    sbSendTelegram[4] = 0x2;
                    sbSendTelegram[5] = 0xe1;
                    sbSendTelegram[6] = 0x0;
                    sbSendTelegram[7] = 0x80 | testValue;
                    testValue ^= 1;

                    sb_send_tel(sbSendTelegram, 8);
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
