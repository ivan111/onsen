// Generated by mkhdr

#ifndef HEADER_INTR
#define HEADER_INTR

#include <stdint.h>

//-----------------------------------------------------------------------------
// 割り込み

void intr_init(void);

void notify_intr_end(unsigned char irq);

void set_pic0_mask(uint8_t mask);
void set_pic1_mask(uint8_t mask);

void intr_enable(int irq);


//-----------------------------------------------------------------------------
// フォールト


#endif
