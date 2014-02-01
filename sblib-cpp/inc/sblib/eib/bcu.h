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
#include <sblib/eib/properties.h>
#include <sblib/eib/user_memory.h>

// Rename the method appData() to indicate the BCU type. If you get a link error
// then the library's BCU_TYPE is different from the application's BCU_TYPE.
#if BCU_TYPE == 10
# define appData appData_BCU1
#elif BCU_TYPE == 20
# define appData appData_BCU2
#else
# error Unsupported BCU_TYPE
#endif


class BCU;

/**
 * The EIB bus coupling unit.
 */
extern BCU bcu;


/**
 * Class for controlling all BCU related things.
 *
 * In order to use the EIB bus, you need to call bcu.begin() once in your application's
 * setup() function.
 */
class BCU
{
public:
    BCU();

    /**
     * Begin using the EIB bus coupling unit, and set the  manufacturer-ID, device type,
     * and program version.
     *
     * @param manufacturer - the manufacturer ID (16 bit)
     * @param deviceType - the device type (16 bit)
     * @param version - the version of the application program (8 bit)
     */
    void begin(int manufacturer, int deviceType, int version);

    /**
     * End using the EIB bus coupling unit.
     */
    void end();

    /**
     * Set our own physical address. Normally the physical address is set by ETS when
     * programming the device.
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
     * The BCU's main processing loop. This is like the application's loop() function,
     * and is called automatically by main() when the BCU is activated with bcu.begin().
     */
    void loop();

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
     */
    void processDirectTelegram(int apci);

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
    Debouncer progButtonDebouncer; //!< The debouncer for the programming mode button.
    bool enabled;                  //!< The BCU is enabled. Set by bcu.begin().
    byte sendCtrlTelegram[8];      //!< A short buffer for connection control telegrams.
    int  connectedAddr;            //!< Remote address of the connected partner.
    int  connectedSeqNo;           //!< Sequence number for connected data telegrams.
    bool incConnectedSeqNo;        //!< True if the sequence number shall be incremented on ACK.
};

//#undef appData


//
//  Inline functions
//

inline bool BCU::programmingMode() const
{
    return (userRam.status & BCU_STATUS_PROG) == BCU_STATUS_PROG;
}

inline bool BCU::applicationRunning() const
{
    if (!enabled)
        return true;

#if BCU_TYPE == 10
    return (userRam.status & (BCU_STATUS_PROG|BCU_STATUS_AL)) == BCU_STATUS_AL &&
        userRam.runState == 1 && userEeprom.runError == 0xff; // ETS sets the run error to 0 when programming
#elif BCU_TYPE >= 20
    return !(userRam.status & BCU_STATUS_PROG) &&
        userRam.runState == 1 && userEeprom.loadState[OT_APPLICATION];
#else
#   error Unsupported BCU_TYPE
#endif
}

inline bool BCU::directConnection() const
{
    return connectedAddr != 0;
}

#endif /*sblib_bcu_h*/
