/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_props_h
#define sb_props_h

/**
 * Process a property-value read telegram.
 *
 * @param objectIdx - the object index
 * @param propertyId - the property ID
 * @param count - the number of elements to read
 * @param start - the index of the first element to read
 */
void sb_props_read_tel(unsigned char objectIdx, unsigned char propertyId,
                       unsigned char count, unsigned short start);

#endif /*sb_props_h*/
