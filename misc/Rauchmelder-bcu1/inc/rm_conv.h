/*
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef rm_conv_h
#define rm_conv_h

//
//  Funktionen für die Umwandlung von Datentypen
//

/**
 * Einen Wert auf DPT 9.001 2 Byte Float Format wandeln
 *
 * @param val - der zu wandelnde Wert
 * @return Der Wert val im DPT 9.001 Format
 */
extern unsigned long conv_dpt_9_001(unsigned long val);

#endif /*rm_conv_h*/
