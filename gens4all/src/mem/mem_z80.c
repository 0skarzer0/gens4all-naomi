/*****************************************************************************/
/* Modulo manejador de memoria del Z80                                       */
/* Autor: Oscar Orallo Pelaez                                                */
/* Fecha: 15-02-2007                                                         */
/*****************************************************************************/

#include <stdio.h>
#include "gens4all.h"
#include "mem_z80.h"
#include "sound/ym2612.h"
#include "vdp/vdp_io.h"


#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define WRONG_ACCESS_DATA 0

u8 Ram_Z80[8 * 1024];
int Bank_Z80;

/* Read tables */

u32 Z80_ReadB_Table[] =
{
	(u32) Z80_ReadB_Ram,		// 0x0000 - 0x0FFF
	(u32) Z80_ReadB_Ram,		// 0x1000 - 0x1FFF
	(u32) Z80_ReadB_Ram,		// 0x2000 - 0x2FFF
	(u32) Z80_ReadB_Ram,		// 0x3000 - 0x3FFF
	(u32) Z80_ReadB_YM2612,		// 0x4000 - 0x4FFF
	(u32) Z80_ReadB_YM2612,		// 0x5000 - 0x5FFF
	(u32) Z80_ReadB_Bank,		// 0x6000 - 0x6FFF
	(u32) Z80_ReadB_PSG,		// 0x7000 - 0x7FFF
	(u32) Z80_ReadB_68K_Ram,	// 0x8000 - 0x8FFF
	(u32) Z80_ReadB_68K_Ram,	// 0x9000 - 0x9FFF
	(u32) Z80_ReadB_68K_Ram,	// 0xA000 - 0xAFFF
	(u32) Z80_ReadB_68K_Ram,	// 0xB000 - 0xBFFF
	(u32) Z80_ReadB_68K_Ram,	// 0xC000 - 0xCFFF
	(u32) Z80_ReadB_68K_Ram,	// 0xD000 - 0xDFFF
	(u32) Z80_ReadB_68K_Ram,	// 0xE000 - 0xEFFF
	(u32) Z80_ReadB_68K_Ram		// 0xF000 - 0xFFFF
};

u32 Z80_ReadW_Table[] =
{
	(u32) Z80_ReadW_Ram,		// 0x0000 - 0x0FFF
	(u32) Z80_ReadW_Ram,		// 0x1000 - 0x1FFF
	(u32) Z80_ReadW_Ram,		// 0x2000 - 0x2FFF
	(u32) Z80_ReadW_Ram,		// 0x3000 - 0x3FFF
	(u32) Z80_ReadW_YM2612,		// 0x4000 - 0x4FFF
	(u32) Z80_ReadW_YM2612,		// 0x5000 - 0x5FFF
	(u32) Z80_ReadW_Bank,		// 0x6000 - 0x6FFF
	(u32) Z80_ReadW_PSG,		// 0x7000 - 0x7FFF
	(u32) Z80_ReadW_68K_Ram,	// 0x8000 - 0x8FFF
	(u32) Z80_ReadW_68K_Ram,	// 0x9000 - 0x9FFF
	(u32) Z80_ReadW_68K_Ram,	// 0xA000 - 0xAFFF
	(u32) Z80_ReadW_68K_Ram,	// 0xB000 - 0xBFFF
	(u32) Z80_ReadW_68K_Ram,	// 0xC000 - 0xCFFF
	(u32) Z80_ReadW_68K_Ram,	// 0xD000 - 0xDFFF
	(u32) Z80_ReadW_68K_Ram,	// 0xE000 - 0xEFFF
	(u32) Z80_ReadW_68K_Ram		// 0xF000 - 0xFFFF
};

/* Write tables */
u32 Z80_WriteB_Table[] =
{
	(u32) Z80_WriteB_Ram,		// 0x0000 - 0x0FFF
	(u32) Z80_WriteB_Ram,		// 0x1000 - 0x1FFF
	(u32) Z80_WriteB_Ram,		// 0x2000 - 0x2FFF
	(u32) Z80_WriteB_Ram,		// 0x3000 - 0x3FFF
	(u32) Z80_WriteB_YM2612,	// 0x4000 - 0x4FFF
	(u32) Z80_WriteB_YM2612,	// 0x5000 - 0x5FFF
	(u32) Z80_WriteB_Bank,		// 0x6000 - 0x6FFF
	(u32) Z80_WriteB_PSG,		// 0x7000 - 0x7FFF
	(u32) Z80_WriteB_68K_Ram,	// 0x8000 - 0x8FFF
	(u32) Z80_WriteB_68K_Ram,	// 0x9000 - 0x9FFF
	(u32) Z80_WriteB_68K_Ram,	// 0xA000 - 0xAFFF
	(u32) Z80_WriteB_68K_Ram,	// 0xB000 - 0xBFFF
	(u32) Z80_WriteB_68K_Ram,	// 0xC000 - 0xCFFF
	(u32) Z80_WriteB_68K_Ram,	// 0xD000 - 0xDFFF
	(u32) Z80_WriteB_68K_Ram,	// 0xE000 - 0xEFFF
	(u32) Z80_WriteB_68K_Ram	// 0xF000 - 0xFFFF
};

u32 Z80_WriteW_Table[] =
{
	(u32) Z80_WriteW_Ram,		// 0x0000 - 0x0FFF
	(u32) Z80_WriteW_Ram,		// 0x1000 - 0x1FFF
	(u32) Z80_WriteW_Ram,		// 0x2000 - 0x2FFF
	(u32) Z80_WriteW_Ram,		// 0x3000 - 0x3FFF
	(u32) Z80_WriteW_YM2612,	// 0x4000 - 0x4FFF
	(u32) Z80_WriteW_YM2612,	// 0x5000 - 0x5FFF
	(u32) Z80_WriteW_Bank,		// 0x6000 - 0x6FFF
	(u32) Z80_WriteW_PSG,		// 0x7000 - 0x7FFF
	(u32) Z80_WriteW_68K_Ram,	// 0x8000 - 0x8FFF
	(u32) Z80_WriteW_68K_Ram,	// 0x9000 - 0x9FFF
	(u32) Z80_WriteW_68K_Ram,	// 0xA000 - 0xAFFF
	(u32) Z80_WriteW_68K_Ram,	// 0xB000 - 0xBFFF
	(u32) Z80_WriteW_68K_Ram,	// 0xC000 - 0xCFFF
	(u32) Z80_WriteW_68K_Ram,	// 0xD000 - 0xDFFF
	(u32) Z80_WriteW_68K_Ram,	// 0xE000 - 0xEFFF
	(u32) Z80_WriteW_68K_Ram	// 0xF000 - 0xFFFF
};

u8 Z80_ReadB_Bad(u32 Adr)
{
	return 0;
}

u8 Z80_ReadB_Ram(u32 Adr)
{
	return Ram_Z80[Adr & 0x1FFF];
}

u8 Z80_ReadB_Bank(u32 Adr)
{
	return 0;
}

u8 Z80_ReadB_YM2612(u32 Adr)
{
	return YM2612_Read(); //Adr & 3);
}

u8 Z80_ReadB_PSG(u32 Adr)
{
	if (Adr < 0x7F04)
	{
		/* Wrong access Data */
		return WRONG_ACCESS_DATA;
	}
	else
	{
		if (Adr < 0x7F08)
		{
			/* VDP status */
			u16 status = Read_VDP_Status();

			/* Test to see if it should be byte-swapped */
			if (Adr & 1)
			{
				return status & 0xFF;
			}
			else
			{
				return ((status & 0xFF00) >> 8);
			}
		}
		else
		{
			if (Adr > 0x7F09)
			{
				/* Wrong access */
				return WRONG_ACCESS_DATA;
			}
			else
			{
				if (Adr & 1)
				{
					return Read_VDP_H_Counter();
				}
				else
				{
					return Read_VDP_V_Counter();
				}
			}
		}
	}
}


u8 Z80_ReadB_68K_Ram(u32 Adr)
{
	return M68K_RB( (Adr & 0x7FFF) + Bank_Z80 );
}


/* Read word */
u16 Z80_ReadW_Bad(u32 Adr)
{
	return 0;
}

u16 Z80_ReadW_Ram(u32 Adr)
{
	u16 retval;

	retval = Ram_Z80[Adr & 0x1FFF];
	retval |= Ram_Z80[(Adr & 0x1FFF) + 1] << 8;
}

u16 Z80_ReadW_Bank(u32 Adr)
{
	return 0;
}

u16 Z80_ReadW_YM2612(u32 Adr)
{
	return YM2612_Read(); //Adr & 3) & 0xFF;
}

u16 Z80_ReadW_PSG(u32 Adr)
{
	if (Adr < 0x7F04)
	{
		/* Wrong access */
		return WRONG_ACCESS_DATA;
	}
	else
	{
		if (Adr < 0x7F08)
		{
			/* VDP status */
			return Read_VDP_Status();
		}
		else
		{
			if (Adr > 0x7F09)
			{
				/* Wrong access */
				return WRONG_ACCESS_DATA;
			}
			else
			{
				return ((Read_VDP_V_Counter() << 8) | Read_VDP_H_Counter());
			}
		}
	}
}

u16 Z80_ReadW_68K_Ram(u32 Adr)
{
	u32 aux_Adr = Bank_Z80 + (Adr & 0xFFFF);

	return (M68K_RB(aux_Adr + 1) << 8) | M68K_RB(aux_Adr);
}


/* Write byte */
void Z80_WriteB_Bad(u32 Adr, u8 Data)
{
}

void Z80_WriteW_Bad(u32 Adr, u16 Data)
{
}

void Z80_WriteB_Ram(u32 Adr, u8 Data)
{
	Ram_Z80[Adr & 0x1FFF] = Data;
}



void Z80_WriteB_Bank(u32 Adr, u8 Data)
{
	if (Adr > 0x60FF)
	{
		/* Wrong access */
		return;
	}
	else
	{
		Bank_Z80 = ((Bank_Z80 & 0xFF0000) >> 1) + ((Data & 1) << 23);
	}
}

void Z80_WriteW_Bank(u32 Adr, u16 Data)
{
	Z80_WriteB_Bank(Adr,Data);
}


void Z80_WriteB_YM2612(u32 Adr, u8 Data)
{
	YM2612_Write(Adr & 3,Data);
}


void Z80_WriteB_PSG(u32 Adr, u8 Data)
{
	if (Adr == 0x7F11)
	{
		PSG_Write(Data);
	}
	else
	{
		if (Adr > 0x7F03)
		{
			/* Wrong access */
			return;
		}
		else
		{
			Write_Byte_VDP_Data(Data);
		}
	}
}


void Z80_WriteB_68K_Ram(u32 Adr, u8 Data)
{
	M68K_WB( (Adr & 0x7FFF) + Bank_Z80, Data);
}

/* Write word */

void Z80_WriteW_Ram(u32 Adr, u16 Data)
{
	Ram_Z80[Adr & 0x1FFF] = Data & 0xFF;
	Ram_Z80[(Adr & 0x1FFF) + 1] = (Data >> 8) & 0xFF;
}

void Z80_WriteW_YM2612(u32 Adr, u16 Data)
{
	YM2612_Write(Adr & 0x3, Data);
	YM2612_Write((Adr & 0x3) + 1, Data);
}

void Z80_WriteW_PSG(u32 Adr, u16 Data)
{
	if (Adr == 0x7F11)
	{
		PSG_Write(Data);
	}
	else
	{
		if (Adr > 0x7F03)
		{
			/* Wrong access */
			return;
		}
		else
		{
			Write_Word_VDP_Data(Data);
		}
	}
}

void Z80_WriteW_68K_Ram(u32 Adr, u16 Data)
{
	M68K_WB((Adr & 0xFFFF) + Bank_Z80, Data & 0xFF);
	M68K_WB((Adr & 0xFFFF) + Bank_Z80 + 1, (Data >> 8) & 0xFF);
}


/*****************************************************************************/

// unsigned char Z80_ReadB(unsigned int Adr);
u8 Z80_ReadB(u32 Adr)
{
	u8 val = ((u8(*)(u32))Z80_ReadB_Table[(Adr & 0xF000) >> 12])(Adr & 0xFFFF);
	return val;
}

// unsigned short Z80_ReadW(unsigned int Adr);
u16 Z80_ReadW(u32 Adr)
{
	u16 val = ((u16(*)(u32))Z80_ReadW_Table[(Adr & 0xF000) >> 12])(Adr & 0xFFFF);
	return val;
}

// void Z80_WriteB(unsigned int Adr, unsigned char Data);
void Z80_WriteB(u32 Adr, u8 Data)
{
	((void(*)(u32,u16))Z80_WriteB_Table[(Adr & 0xF000) >> 12])(Adr & 0xFFFF, Data);
}

// void Z80_WriteW(unsigned int Adr, unsigned short Data);
void Z80_WriteW(u32 Adr, u16 Data)
{
	((void(*)(u32,u16))Z80_WriteW_Table[(Adr & 0xF000) >> 12])(Adr & 0xFFFF, Data);
}
