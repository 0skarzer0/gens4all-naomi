#ifndef FAME
#include "starscream/star_68k.h"
#else
#include "fame/fame.h"
#endif

#ifndef _CPU_68K_H
#define _CPU_68K_H

#ifndef FAME
extern struct S68000CONTEXT Context_68K;
#endif

int M68K_Init();
int S68K_Init();
void M68K_Reset(int System_ID);
void S68K_Reset();

#ifdef FAME
#define M68K_Reset_CPU() m68k_reset()
#define M68K_GetCyclesDone() m68k_get_cycles_counter()
#define M68K_Interrupt(LVL) m68k_raise_irq((LVL),M68K_AUTOVECTORED_IRQ)
#define M68K_GetPC() m68k_get_pc()
#define M68K_ResetCyclesCounter() m68k_trip_cycles_counter()
#define M68K_ReleaseCycles() m68k_release_timeslice()
#define M68K_AddCycles(CL) m68k_add_cycles((CL))
#else
#define M68K_Reset_CPU() main68k_reset()
#define M68K_GetCyclesDone() main68k_readOdometer()
#define M68K_Interrupt(LVL) main68k_interrupt((LVL),-1)
#define M68K_GetPC() main68k_readPC()
#define M68K_ResetCyclesCounter() main68k_tripOdometer()
#define M68K_ReleaseCycles() main68k_releaseCycles()
#define M68K_AddCycles(CL) main68k_addCycles((CL))
#endif

#ifdef SEGA_CD
#define S68K_Reset_CPU() sub68k_reset()
#endif

#if INLINE68K
#include "cpu/m68k/fame/fame.h"
#include "profiler.h"
static inline void M68K_Exec(int cycles)
{
	prof_start(PROF_M68K);
	extern M68K_CONTEXT m68kcontext;
	if ((cycles - m68kcontext.cycles_counter) > 0)
		m68k_emulate(cycles - m68kcontext.cycles_counter);

	prof_end(PROF_M68K);
}
#else
void M68K_Exec(int cycles);
#endif

void M68K_32X_Mode();
void M68K_Set_32X_Rom_Bank();
void M68K_Set_Prg_Ram();
void MS68K_Set_Word_Ram();

#endif
