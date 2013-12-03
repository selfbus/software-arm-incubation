#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CONFIG_ENABLE_DRIVER_GPIO 1

#include "../driver/gpio.h"

#include "sb_bus.h"


#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led



int main(void)
{
	int i, on = 0;

	sb_init_bus();

	GPIOSetDir(LED_PORT, LED_BIT, 1);
    GPIOSetValue(LED_PORT, LED_BIT, 0);

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
	}

	return 0;
}
