/*
 *  prog_uart.h - Soft-Uart of the User Programming interface
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef prog_uart_h
#define prog_uart_h

#include <sblib/timer.h>

enum class TProgUartErr
{
    Ok,
    Error,
    Busy
};

class ProgUart
{
public:
    ProgUart(Timer& aTimer, int aTimerNum, int aRxPin, int aTxPin, TimerCapture arx_CaptureCh, TimerMatch arx_MatchCh, TimerMatch atx_MatchCh, int aIspEnPin, int aIspRstPin);
    void SerIf_Tasks(void);
    void timerInterruptHandler();
    void Enable(void);
    void Disable(void);
    void SetIspLines(uint8_t data);

protected:
    Timer& timer;                //!< The timer
    int timerNum;
    int rxPin, txPin;            //!< The pins for bus receiving and sending
    TimerCapture rx_captureCh;   //!< The timer channel that captures the timer value on the bus-in pin
    TimerMatch rx_matchCh;
    TimerMatch tx_matchCh;
    int rxbitcnt;
    int txbitcnt;
    uint8_t rxbyte;
    uint8_t txbyte;
    int txbuffno;
    int rxbuffno;
    uint8_t *txptr;
    uint8_t *rxptr;
    int txlen;
    int rxlen;

    int IspEnPin;
    int IspRstPin;
    bool Enabled;
    uint8_t IspLines;
    void UpdIspLines(void);

private:
    void EnableUart();
    void DisableUart();
    bool TxBusy(void);
    bool rxBusy(void);
    TProgUartErr TransmitBuffer(int buffno);
    void initialize();

    /**
     * Validates if a given buffer adheres to UUencoding rules.
     *
     * @details This method checks whether each byte in the provided buffer is valid
     *          according to UUencoding specifications. Valid characters for UUencoding
     *          are within the range of ASCII ' ' (0x20) to '`' (0x60). It also
     *          accepts characters which hold special significance in contexts of
     *          ISP autobaud synchronization and escape sequences.
     *
     * @param buffer Pointer to the input buffer containing bytes to validate.
     * @param length The length of the input buffer.
     * @return true if the buffer is valid UUencoded data, false otherwise.
     *
     * @note The following characters are explicitly ignored during validation:
     *       - Line feed (0x0d)
     *       - Escape character (<ESC>)
     *       - Lower case letters of the word "Synchronized"
     */
    static bool isValidUUEncoding(uint8_t * buffer, uint32_t length);
};


#endif
