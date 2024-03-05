/*****************************************************************************
 *   timer32.h:  Header file for NXP LPC1xxx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.08.20  ver 1.00    Preliminary version, first Release
 *   2010.03.24      1.01    Minor weaks by Code Red
 *
******************************************************************************/
#ifndef __TIMER32_H 
#define __TIMER32_H

#ifdef __cplusplus
extern "C" {
#endif

void delay32Ms(uint8_t timer_num, uint32_t delayInMs);
void TIMER32_0_IRQHandler(void);
void TIMER32_1_IRQHandler(void);
void enable_timer32(uint8_t timer_num);
void disable_timer32(uint8_t timer_num);
void reset_timer32(uint8_t timer_num);
void init_timer32(uint8_t timer_num, uint32_t timerInterval);

#ifdef __cplusplus
}; // this brace closes the extern "C" declaration
#endif

#endif /* end __TIMER32_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
