/*
 *  app-rol-jal.cpp - The application for the 4 channel blinds/shutter actuator
 *  acting as a Jung 2204REGH
 *
 *  Copyright (C) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "config.h"
#include "app-rol-jal.h"

#if OUTPUTTYPE == MONO
#include "MonoOutputs.h"
#endif


void objectUpdated(int objno)
{

}

void checkPeriodicFuntions(void)
{

}

void initApplication(void)
{
    outputs.init();
}
