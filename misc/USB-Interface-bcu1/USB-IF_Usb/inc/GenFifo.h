/*
 *  GenFifo.h - A generic fifo, used to pass buffer numbers from a receiver to a transmitter
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef GENFIFO_H_
#define GENFIFO_H_

#define FIFO_DEPTH 5

enum class TFifoErr
{
	Ok,
	Error,
	Full,
	Empty
};

template <class T, int depth=FIFO_DEPTH>
class GenFifo
{
public:
	GenFifo(void);
	void Purge(void);
	TFifoErr Push(T val);
	TFifoErr Pop(T &val);
	TFifoErr Empty(void);
	TFifoErr Full(void);
	int Level(void);
protected:
	int data[depth+1];
	int rdptr;
	int wrptr;
	int NextPtr(int ptr);
};

extern GenFifo<int> ser_txfifo;
extern GenFifo<int> cdc_txfifo;
extern GenFifo<int> hid_txfifo;
extern GenFifo<int> dev_rxfifo;

#endif /* GENFIFO_H_ */
