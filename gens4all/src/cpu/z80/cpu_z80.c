

#include "gens4all.h"

#ifdef RAZE
#include "raze/raze.h"

int raze_clock_counter;
#endif

//#define Z80_DEBUG


/* Prints Z80 CPU register set */
void show_z80_status(void)
{
#ifdef RAZE
	printf(" AF:0x%04X  AF2: 0x%04X\n",z80_get_reg(Z80_REG_AF),z80_get_reg(Z80_REG_AF2));
	printf(" BC:0x%04X  BC2: 0x%04X\n",z80_get_reg(Z80_REG_BC),z80_get_reg(Z80_REG_BC2));
	printf(" DE:0x%04X  DE2: 0x%04X\n",z80_get_reg(Z80_REG_DE),z80_get_reg(Z80_REG_DE2));
	printf(" HL:0x%04X  HL2: 0x%04X\n",z80_get_reg(Z80_REG_HL),z80_get_reg(Z80_REG_HL2));
	printf(" IX:0x%04X\n",z80_get_reg(Z80_REG_IX));
	printf(" IY:0x%04X\n",z80_get_reg(Z80_REG_IY));
	printf(" PC:0x%04X\n",z80_get_reg(Z80_REG_PC));
#else
	printf(" AF:0x%04X AF2:0x%04X\n",z80_Get_AF(&M_Z80),M_Z80.AF2);
	printf(" BC:0x%04X BC2:0x%04X\n",M_Z80.BC,M_Z80.BC2);
	printf(" DE:0x%04X DE2:0x%04X\n",M_Z80.DE,M_Z80.DE2);
	printf(" HL:0x%04X HL2:0x%04X\n",M_Z80.HL,M_Z80.HL2);
	printf(" IX:0x%04X\n",M_Z80.IX);
	printf(" IY:0x%04X\n",M_Z80.IY);
	printf(" PC:0x%04X\n",z80_Get_PC(&M_Z80));
#endif
}



/*** Z80_Reset - reset z80 sub-unit ***/
void Z80_Reset (void)
{
  // Reset Z80

  memset (Ram_Z80, 0, 8 * 1024);

//      Bank_Z80 = 0x000000;
  Bank_Z80 = 0xFF8000;

  Z80_State &= 1;
//      Z80_State |= 2;         // RZWL needs it but Breath of Fire 3 don't want it...

  Last_BUS_REQ_Cnt = 0;
  Last_BUS_REQ_St = 0;

#ifdef RAZE
	z80_reset();
#else
  z80_Reset (&M_Z80);
#endif	
}

void Z80_IRQ(void)
{
#ifdef RAZE
	z80_raise_IRQ(0xff);
#else
  z80_Interrupt (&M_Z80, 0xFF);
#endif
}

void Z80_Clear_Clock(void)
{
#ifdef RAZE
	raze_clock_counter = 0;
#else
  z80_Clear_Odo (&M_Z80);
#endif
}

void Z80_Set_Clock(int cycles)
{
#ifdef RAZE
	if (cycles < 0)	{
		printf("Weird cycles: %i from %p\n", cycles, __builtin_return_address(0)); fflush(stdout);
	}
	raze_clock_counter = cycles;
#else
  z80_Set_Odo (&M_Z80,cycles);
#endif
}

/*** Z80_Init - initialise this sub-unit ***/

int Z80_Init (void)
{
#ifdef RAZE
	z80_init_memmap();

	z80_map_fetch(0x0000, 0x1fff, &Ram_Z80[0]);
	z80_map_fetch(0x8000, 0x3fff, &Ram_Z80[0]);

	z80_add_read(0x0000, 0x3fff, Z80_MAP_HANDLED, (void*)Z80_ReadB_Ram);
	z80_add_read(0x4000, 0x5fff, Z80_MAP_HANDLED, (void*)Z80_ReadB_YM2612);
	z80_add_read(0x6000, 0x6fff, Z80_MAP_HANDLED, (void*)Z80_ReadB_Bank);
	z80_add_read(0x7000, 0x7fff, Z80_MAP_HANDLED, (void*)Z80_ReadB_PSG);
	z80_add_read(0x8000, 0xffff, Z80_MAP_HANDLED, (void*)Z80_ReadB_68K_Ram);

	z80_add_write(0x0000, 0x3fff, Z80_MAP_HANDLED, (void*)Z80_WriteB_Ram);
	z80_add_write(0x4000, 0x5fff, Z80_MAP_HANDLED, (void*)Z80_WriteB_YM2612);
	z80_add_write(0x6000, 0x6fff, Z80_MAP_HANDLED, (void*)Z80_WriteB_Bank);
	z80_add_write(0x7000, 0x7fff, Z80_MAP_HANDLED, (void*)Z80_WriteB_PSG);
	z80_add_write(0x8000, 0xffff, Z80_MAP_HANDLED, (void*)Z80_WriteB_68K_Ram);

	z80_end_memmap();
	z80_reset();
#else
  // Init Z80

  z80_Init (&M_Z80);

  z80_Add_Fetch (&M_Z80, 0x00, 0x1F, &Ram_Z80[0]);
  z80_Add_Fetch (&M_Z80, 0x20, 0x3F, &Ram_Z80[0]);

  z80_Add_ReadB (&M_Z80, 0x00, 0xFF, (Z80_RB *)Z80_ReadB_Bad);
  z80_Add_ReadB (&M_Z80, 0x00, 0x3F, (Z80_RB *)Z80_ReadB_Ram);
  z80_Add_ReadB (&M_Z80, 0x40, 0x5F, (Z80_RB *)Z80_ReadB_YM2612);
  z80_Add_ReadB (&M_Z80, 0x60, 0x6F, (Z80_RB *)Z80_ReadB_Bank);
  z80_Add_ReadB (&M_Z80, 0x70, 0x7F, (Z80_RB *)Z80_ReadB_PSG);
  z80_Add_ReadB (&M_Z80, 0x80, 0xFF, (Z80_RB *)Z80_ReadB_68K_Ram);

  z80_Add_ReadW (&M_Z80, 0x00, 0xFF, (Z80_RW *)Z80_ReadW_Bad);
  z80_Add_ReadW (&M_Z80, 0x00, 0x3F, (Z80_RW *)Z80_ReadW_Ram);
  z80_Add_ReadW (&M_Z80, 0x40, 0x5F, (Z80_RW *)Z80_ReadW_YM2612);
  z80_Add_ReadW (&M_Z80, 0x60, 0x6F, (Z80_RW *)Z80_ReadW_Bank);
  z80_Add_ReadW (&M_Z80, 0x70, 0x7F, (Z80_RW *)Z80_ReadW_PSG);
  z80_Add_ReadW (&M_Z80, 0x80, 0xFF, (Z80_RW *)Z80_ReadW_68K_Ram);

  z80_Add_WriteB (&M_Z80, 0x00, 0xFF, (Z80_WB *)Z80_WriteB_Bad);
  z80_Add_WriteB (&M_Z80, 0x00, 0x3F, (Z80_WB *)Z80_WriteB_Ram);
  z80_Add_WriteB (&M_Z80, 0x40, 0x5F, (Z80_WB *)Z80_WriteB_YM2612);
  z80_Add_WriteB (&M_Z80, 0x60, 0x6F, (Z80_WB *)Z80_WriteB_Bank);
  z80_Add_WriteB (&M_Z80, 0x70, 0x7F, (Z80_WB *)Z80_WriteB_PSG);
  z80_Add_WriteB (&M_Z80, 0x80, 0xFF, (Z80_WB *)Z80_WriteB_68K_Ram);

  z80_Add_WriteW (&M_Z80, 0x00, 0xFF, (Z80_WW *)Z80_WriteW_Bad);
  z80_Add_WriteW (&M_Z80, 0x00, 0x3F, (Z80_WW *)Z80_WriteW_Ram);
  z80_Add_WriteW (&M_Z80, 0x40, 0x5F, (Z80_WW *)Z80_WriteW_YM2612);
  z80_Add_WriteW (&M_Z80, 0x60, 0x6F, (Z80_WW *)Z80_WriteW_Bank);
  z80_Add_WriteW (&M_Z80, 0x70, 0x7F, (Z80_WW *)Z80_WriteW_PSG);
  z80_Add_WriteW (&M_Z80, 0x80, 0xFF, (Z80_WW *)Z80_WriteW_68K_Ram);

  Z80_Reset ();
#endif

  return 1;
}


void Z80_Process(int cycles)
{
	prof_start(PROF_Z80);
#ifdef RAZE
	if (Z80_State == 3)
	{
		int cycles = (Cycles_Z80 - raze_clock_counter);
		if (cycles > 0)
		{
			int oldcounter=raze_clock_counter;
			if (oldcounter < 0) {
				printf("Counter issue %i\n", oldcounter); fflush(stdout);
			}
			raze_clock_counter += z80_emulate(cycles);
			if (oldcounter == raze_clock_counter) {
				printf("Counter issue\n"); fflush(stdout);
			}
			z80_lower_IRQ();
		}
	}
	else
	{
		raze_clock_counter = Cycles_Z80;
	}
#else
#ifdef __RESULT__
      if (Z80_State == 3)
	{
		asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
	}
      else
	{
	  z80_Set_Odo (&M_Z80, Cycles_Z80);
	}
#else
      if (Z80_State == 3)
	{
#ifdef Z80_DEBUG
			int i;
			int ciclos=Cycles_Z80/8;

			if (ciclos < 0) ciclos = 0;
			
			for(i=0;i<ciclos;i++)
			{
				show_z80_status();
				z80_Exec (&M_Z80, 1 + M_Z80.CycleCnt);
			}
#else
	z80_Exec (&M_Z80, Cycles_Z80);
#endif

	}
      else
	z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
#endif
	prof_end(PROF_Z80);
}


#include "cd_sys.h"


void Write_To_Bank (int val)
{
#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "Z80 write bank : %d     Bank = %.8X\n", val,
	   Bank_Z80);
#endif
}

void Read_To_68K_Space (int adr)
{
#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "Z80 read in 68K space : $(%.8X)\n", adr);
#endif
}

void Write_To_68K_Space (int adr, int data)
{
#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "Z80 write in 68K space : $(%.8X) = %.8X\n", adr,
	   data);
#endif
}
