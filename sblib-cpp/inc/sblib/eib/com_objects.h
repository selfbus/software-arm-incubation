/*
 *  com_objects.h - EIB Communication objects.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_com_objects_h
#define sblib_com_objects_h

#include <sblib/eib/bcu.h>
#include <sblib/eib/types.h>


/**
 * Get the numeric value from a communication object. Can be used for
 * communication objects of type 1 bit up to type 4 bytes.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object.
 */
unsigned int objectRead(int objno);

/**
 * Get the float value from a communication object. Can be used for
 * communication objects of type FLOAT.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object.
 */
float objectReadFloat(int objno);

/**
 * Get a pointer to the value bytes of the communication object. Can be used for
 * any communication object. The minimum that is used for a communication object
 * is 1 byte. Use objectSize(objno) to get the size of the communication object's
 * value.
 *
 * @param objno - the ID of the communication object.
 * @return The value of the com-object.
 */
byte* objectValuePtr(int objno);

/**
 * Get the size of the communication object's value in bytes.
 *
 * @param objno - the ID of the communication object.
 * @return The size in bytes.
 */
int objectSize(int objno);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectWrite(int objno, unsigned int value);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectWrite(int objno, float value);

/**
 * Set the value of a communication object. Calling this function triggers the
 * sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 * @param value - the new value of the communication object.
 */
void objectWrite(int objno, byte* value);

/**
 * Mark a communication object as written. Use this function if you directly change
 * the value of a communication object without using objectWrite(). Calling this
 * function triggers the sending of a write-group-value telegram.
 *
 * @param objno - the ID of the communication object.
 */
void objectWritten(int objno);

/**
 * Get the ID of the next communication object that was updated
 * over the bus by a write-value-request telegram.
 *
 * @return The ID of the next updated com-object, -1 if none.
 */
int nextUpdatedObject();

/**
 * Get the type of a communication object.
 *
 * @param objno - the ID of the communication object.
 * @return The type of the communication object.
 */
ComType objectType(int objno);

/**
 * Get the communication object configuration.
 *
 * @param objno - the ID of the communication object
 * @return The communication object configuration.
 */
const ComConfig& objectConfig(int objno);

/**
 * Set one or more flags of a communication object.
 * This does not change the other flags.
 *
 * @param objno - the ID of the communication object
 * @param flags - the flags to set
 */
void setObjectFlags(int objno, int flags);

/**
 * Process a multicast group telegram.
 *
 * This function is called by bcu.processTelegram(). It is usually not required to call
 * this function from within a user program. The telegram that is processed is read from
 * bus.telegram[].
 *
 * @param addr - the destination group address.
 */
void processGroupTelegram(int addr, int apci);

/**
 * Get the communication object configuration table ("COMMS" table). This is the table
 * with the flags that are configured by ETS (not the RAM status flags).
 *
 * @return The com-objects configuration table.
 *
 * @brief The first byte of the table contains the number of entries. The second
 * byte contains the address of the object status flags in userRam. The rest of
 * the table consists of the ComConfig objects - 3 bytes per communication
 * object.
 */
byte* objectConfigTable();

/**
 * Get the communication object status flags table. This is the table with the
 * status flags that are stored in RAM and get changed during normal operation.
 *
 * @return The com-objects status flags table.
 *
 * @brief The whole table consists of the status flags - 4 bits per communication
 * object.
 */
byte* objectFlagsTable();


//
//  Inline functions
//

inline ComType objectType(int objno)
{
    return (ComType) objectConfig(objno).type;
}

inline const ComConfig& objectConfig(int objno)
{
    return *(const ComConfig*) (objectConfigTable() + objno * 3 + 2);
}

inline void objectWritten(int objno)
{
    setObjectFlags(objno, COMFLAG_TRANSREQ);
}

#endif /*sblib_com_objects_h*/
