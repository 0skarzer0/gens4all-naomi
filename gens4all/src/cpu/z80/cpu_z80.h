#ifndef _CPU_Z80_H
#define _CPU_Z80_H

int Z80_Init(void);
void Z80_Reset(void);
void Z80_Clear_Clock(void);
void Z80_Process(int cycles);
void Z80_IRQ(void);
void Z80_Set_Clock(int cycles);

#endif
