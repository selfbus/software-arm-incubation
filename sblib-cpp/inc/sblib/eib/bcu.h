/*
 *  bcu.h - BCU specific stuff.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bcu_h
#define sblib_bcu_h

#include <sblib/types.h>
#include <sblib/eib/bus.h>
#include <sblib/eib/bcu_type.h>
#include <sblib/eib/user_memory.h>

class BCU;
class BUS;


/**
 * The EIB bus coupling unit.
 */
extern BCU bcu;


/**
 * Class for controlling all BCU related things.
 */
class BCU
{
public:
    /**
     * Begin using the EIB bus coupling unit.
     */
    void begin();

    /**
     * End using the EIB bus coupling unit.
     */
    void end();

    /**
     * Set manufacturer data, manufacturer-ID, and device type.
     *
     * @param data - the manufacturer data (16 bit)
     * @param manufacturer - the manufacturer ID (16 bit)
     * @param deviceType - the device type (16 bit)
     * @param version - the version of the application program (8 bit)
     */
    void appData(int data, int manufacturer, int deviceType, byte version);

    /**
     * Set our own physical address
     *
     * @param addr - the physical address
     */
    void setOwnAddress(int addr);

    /**
     * Test if the programming mode is active. This is also indicated
     * by the programming mode LED.
     *
     * @return True if the programming mode is active, false if not.
     */
    bool programmingMode() const;

    /**
     * Test if the user application is active. The application is active if the
     * application layer is active in userRam.status, the programming mode is not
     * active, and the run error in userEeprom.runError is 0xff (no error).
     *
     * @return True if the user application is active, false if not.
     */
    bool applicationRunning() const;

    /**
     * Test if a direct data connection is open.
     *
     * @return True if a connection is open, false if not.
     */
    bool directConnection() const;

    /**
     * Process the received telegram from bus.telegram.
     * Called by main()
     */
    void processTelegram();

    /**
     * A buffer for sending telegrams. This buffer is considered library private
     * and should rather not be used by the application program.
     */
    byte sendTelegram[Bus::TELEGRAM_SIZE];

    /**
     * The pin where the programming LED + button are connected. The default pin
     * is PIO1_5. This variable may be changed in setup(), if required. If set
     * to 0, the programming LED + button are not handled by the library.
     */
    int progPin;

protected:
    /**
     * Process a unicast connection control telegram with our physical address as
     * destination address. The telegram is stored in sbRecvTelegram[].
     *
     * When this function is called, the sender address is != 0 (not a broadcast).
     *
     * @param tpci - the transport control field
     */
    void processConControlTelegram(int tpci);

    /**
     * Process a unicast telegram with our physical address as destination address.
     * The telegram is stored in sbRecvTelegram[].
     *
     * When this function is called, the sender address is != 0 (not a broadcast).
     *
     * @param apci - the application control field
     * @param senderSeqNo - the sequence number of the sender
     */
    void processDirectTelegram(int apci, int senderSeqNo);

    /**
     * Send a connection control telegram.
     *
     * @param cmd - the transport command, see SB_T_xx defines
     * @param senderSeqNo - the sequence number of the sender, 0 if not required
     */
    void sendConControlTelegram(int cmd, int senderSeqNo);

    /**
     * Process a device-descriptor-read request.
     *
     * @param id - the device-descriptor type ID
     *
     * @return True on success, false on failure
     */
    bool processDeviceDescriptorReadTelegram(int id);

private:
    byte sendCtrlTelegram[8];   //!< A short buffer for connection control telegrams.
    int  connectedAddr;         //!< Remote address of the connected partner.
    int  connectedSeqNo;        //!< Sequence number for connected data telegrams.
    bool incConnectedSeqNo;     //!< True if the sequence number shall be incremented on ACK.
};


//
//  Inline functions
//

inline bool BCU::programmingMode() const
{
    return (userRam.status & BCU_STATUS_PROG) == BCU_STATUS_PROG;
}

inline bool BCU::applicationRunning() const
{
    return (userRam.status & (BCU_STATUS_PROG|BCU_STATUS_AL)) == BCU_STATUS_AL &&
        userEeprom.runError == 0xff; // ETS sets the run error to 0 when programming
}

inline bool BCU::directConnection() const
{
    return connectedAddr != 0;
}

#endif /*sblib_bcu_h*/
