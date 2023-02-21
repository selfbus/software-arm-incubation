/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HELPERFUNCTIONS_H_
#define HELPERFUNCTIONS_H_

#include <sblib/eib/bcu_base.h>
#include <sblib/eib/typesBCU2.h>

class HelperFunctions {
public:
	static void setComObjPtr(BcuBase* bcu, int objno, ComType type, uint16_t& addr);
	static void setFlagsTablePtr(BcuBase* bcu, uint16_t flagsAddr);
};

#endif /* HELPERFUNCTIONS_H_ */
