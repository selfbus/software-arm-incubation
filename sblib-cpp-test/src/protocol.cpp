/*
 *  protocol.cpp - protocol tests
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "sblib/internal/variables.h"

void executeTest(Test_Case * tc)
{
    Telegram * tel = tc->telegram;
    unsigned int tn = 1;
    void * refState = &tc->states[0];
    void * stepState = &tc->states[1];

    IAP_Init_Flash(0xFF);
    bcu.begin(tc->manufacturer, tc->deviceType, tc->version);
    if (tc->setup) tc->setup();
    if (tc->gatherState) tc->gatherState(refState, NULL);
    while (tel->type != END)
    {
        // clear the "interrupts" to allow sending of a new telegram
        LPC_TMR16B1->IR = 0x00;
        INFO("Step " << tn << " of test case " << tc->name);
        if (TEL_RX == tel->type)
        {
            unsigned int s = 0;
            memcpy(bus.telegram, tel->bytes, tel->length);
            bus.telegramLen = tel->length;
            bcu.processTelegram();
            if (bus.sendCurTelegram) s++;
            if (bus.sendNextTel) s++;
            REQUIRE(s == tel->variable);
            REQUIRE(bus.telegramLen == 0);
        }
        else if (TEL_TX == tel->type)
        {
            int i;
            int c = 0;
            char msg[1025];
            char numbers[23 * 3 + 1] = { 0 };
            char received[23 * 3 + 1] = { 0 };
            char expected[23 * 3 + 1] = { 0 };
            char temp[1025];
            bus.timerInterruptHandler();
            snprintf(msg, 1024, "%s: Number of bytes in send telegram %d expected %d, sent %d", tc->name, tn,
                tel->length, bus.sendTelegramLen - 1);
            INFO(msg);
            REQUIRE(tel->length == (bus.sendTelegramLen - 1));

            snprintf(msg, 1024, "%s: Send telegram %d mismatch at byte(s) ", tc->name, tn);
            for (i = 0; i < tel->length; i++)
            {
                snprintf(temp, 255, " %2d", i + 1);
                strcat(numbers, temp);
                snprintf(temp, 255, " %02x", bus.sendCurTelegram[i]);
                strcat(received, temp);
                snprintf(temp, 255, " %02x", tel->bytes[i]);
                strcat(expected, temp);
                if (tel->bytes[i] != bus.sendCurTelegram[i])
                {
                    c++;
                    snprintf(temp, 1024, "%d, ", i + 1);
                    strcat(msg, temp);
                }
            }
            msg[strlen(msg) - 2] = '\n'; // remove the last ', ' sequence
            snprintf(temp, 1024, "          %s\n expected: %s\n sent:     %s", numbers, expected, received);
            strcat(msg, temp);
            INFO(msg);
            REQUIRE(c == 0);

            bus.currentByte = SB_BUS_ACK;
            bus.nextByteIndex = 1;
            bus.handleTelegram(true);
            REQUIRE(bus.sendNextTel == NULL);
        }
        else if (TIMER_TICK == tel->type)
        {
            systemTime += tel->length;
        }
        if (tel->stepFunction) tel->stepFunction(refState);
        if (tc->gatherState) tc->gatherState(stepState, refState);
        tn++;
        tel++;
    }
}
