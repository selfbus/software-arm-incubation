// main.c

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "../driver/gpio.h"

#include "sb_bus.h"
#include "sb_proto.h"

#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led

#ifdef EEP_TEST
extern void sb_eep_test (void);
#endif

int main(void)
{
    int i = 0, on = 0;
    int sendWait = 0;

    // Set our own bus address (0x117e == 1.1.126)
    sbOwnPhysicalAddr = 0x1164;

    sb_init_bus();
#ifdef EEP_TEST
    sb_eep_test();
#endif

    GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);

    sbSendTelegram[0] = 0xbc;
    sbSendTelegram[1] = 0x0;  // 1+2: source address: will be set by sb_send_tel() to sbOwnPhysicalAddr
    sbSendTelegram[2] = 0x0;
    sbSendTelegram[3] = 0x10; // 3+4: dest address: 2/0/0
    sbSendTelegram[4] = 0x0;
    sbSendTelegram[5] = 0xe1;
    sbSendTelegram[6] = 0x0;
    sbSendTelegram[7] = 0x80;

    while (1)
    {
        if (sbRecvTelegramLen)
        {
            sb_process_tel();
            GPIOSetValue(LED_PORT, LED_BIT, 1);
            i = 0;
        }

        i = (i + 1) & 0x1fffff;

        if (!i)
        {
            on = !on;
            GPIOSetValue(LED_PORT, LED_BIT, 0);
        }

        if (sendWait > 0)
            --sendWait;
        else if (GPIOGetValue(1, 11) == 0)
        {
            GPIOSetValue(LED_PORT, LED_BIT, 1);
            sendWait = 0x2fffff;
            sb_send_tel(8);
        }
    }

    return 0;
}
