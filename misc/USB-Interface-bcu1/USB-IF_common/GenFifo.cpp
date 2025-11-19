/*
 *  GenFifo.cpp - A generic fifo, used to pass buffer numbers from a receiver to a transmitter
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <string.h>
#include <stdio.h>
#include <GenFifo.h>
#include "BufferMgr.h"

GenFifo<int> ser_txfifo;
GenFifo<int> cdc_txfifo;
GenFifo<int> hid_txfifo;
GenFifo<int> dev_rxfifo;

template <class T, int depth> GenFifo<T, depth>::GenFifo(void)
{
    rdptr = 0;
    wrptr = 0;
}

template <class T, int depth> void GenFifo<T, depth>::Purge(void)
{
    rdptr = 0;
    wrptr = 0;
}

template <class T, int depth> TFifoErr GenFifo<T, depth>::Empty(void)
{
    if (rdptr == wrptr)
        return TFifoErr::Empty;
    else
        return TFifoErr::Ok;
}

template <class T, int depth> TFifoErr GenFifo<T, depth>::Full(void)
{
    if (NextPtr(wrptr) == rdptr)
        return TFifoErr::Full;
    else
        return TFifoErr::Ok;
}

template <class T, int depth> TFifoErr GenFifo<T, depth>::Push(T val)
{
    if (Full() == TFifoErr::Full)
        return TFifoErr::Full;
    data[wrptr] = val;
    wrptr = NextPtr(wrptr);
    return TFifoErr::Ok;
}

template <class T, int depth> TFifoErr GenFifo<T, depth>::Pop(T &val)
{
    if (Empty() == TFifoErr::Empty)
        return TFifoErr::Empty;
    val = data[rdptr];
    rdptr = NextPtr(rdptr);
    return TFifoErr::Ok;
}

template <class T, int depth> int GenFifo<T, depth>::Level(void)
{
    int i;
    i = wrptr - rdptr;
    if (i < 0)
    {
        i += depth+1;
    }
    return i;
}

template <class T, int depth> int GenFifo<T, depth>::NextPtr(int ptr)
{
    ptr++;
    if (ptr > depth)
        return 0;
    else
        return ptr;
}

template class GenFifo<int>;
// Jeder spaeter benutzte Typ wird hier aufgefuehrt
// Siehe z.B. https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor
