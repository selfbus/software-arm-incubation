#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "../driver/gpio.h"

#include "sb_bus.h"
#include "sb_proto.h"


#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led



int main(void)
{
	int i = 0, on = 0;
	int sendWait = 0;

	// Set our own bus address to 1.1.126
	sbOwnPhysicalAddr = 0x117e;

	sb_init_bus();

	GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);

    sbSendTelegram[0] = 0xbc;
    sbSendTelegram[1] = 0x11;
    sbSendTelegram[2] = 0x64;
    sbSendTelegram[3] = 0x0;
    sbSendTelegram[4] = 0x10;

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
		    sb_send_tel(5);
		}
	}

	return 0;
}
