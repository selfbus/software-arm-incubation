/*
 *  internal/functions.h - Library internal shared functions
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_internal_functions_h
#define sblib_internal_functions_h

// Read userEeprom from Flash. (user_memory.cpp)
void readUserEeprom();

// Write userEeprom to Flash. (user_memory.cpp)
void writeUserEeprom();

/*
 * Send the next communication object that is flagged to be sent.
 */
void sendNextGroupTelegram();

/*
 * Process a property-value read telegram. (properties.cpp)
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to read
 * @param start - the index of the first element to read
 *
 * @return True if the request was processed, false if objectIdx or propertyId are
 *         unsupported.
 */
bool propertiesReadTelegram(int objectIdx, PropertyID propertyId, int count, int start);

/*
 * Process a property-value write telegram. (properties.cpp)
 * The data to be written is stored in sbRecvTelegram[12+].
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to write
 * @param start - the index of the first element to write
 *
 * @return True if the request was processed, false if objectIdx or propertyId are
 *         unsupported or the property is not writable.
 */
bool propertiesWriteTelegram(int objectIdx, PropertyID propertyId, int count, int start);

#endif /*sblib_internal_functions_h*/
