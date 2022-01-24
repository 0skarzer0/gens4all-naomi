/*****************************************************************************/
/* Modulo manejador de memoria para el 68000 (CPU principal)                 */
/* Autor: Oscar Orallo Pelaez                                                */
/* Fecha: 08-02-2007                                                         */
/*****************************************************************************/
#include <stdio.h>
#include "gens4all.h"
#include "mem_m68k.h"
#include "cpu_m68k.h"
#include "mem_z80.h"
#include "cpu_z80.h"
#include "vdp_io.h"
#include "io.h"

#ifdef RAZE
#include "raze/raze.h"
#else
#include "stef/z80.h"
#endif

#define GENESIS 0
#define SEGACD  1
#define S32X    2

#define CYCLE_FOR_TAKE_Z80_BUS_GENESIS 16
#define CYCLE_FOR_TAKE_Z80_BUS_SEGACD  32

#define WRONG_ACCESS_DATA 0

#ifndef u8
#define u8 unsigned char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef INLINE
#define INLINE __inline__
#endif


/* function prototypes */

/* Read byte */
INLINE u8 M68K_Read_Byte_Rom0(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom1(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom2(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom3(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom4(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom5(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom6(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom7(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom8(const u32 addr);
INLINE u8 M68K_Read_Byte_Rom9(const u32 addr);
INLINE u8 M68K_Read_Byte_RomA(const u32 addr);
INLINE u8 M68K_Read_Byte_RomB(const u32 addr);
INLINE u8 M68K_Read_Byte_RomC(const u32 addr);
INLINE u8 M68K_Read_Byte_RomD(const u32 addr);
INLINE u8 M68K_Read_Byte_RomE(const u32 addr);
INLINE u8 M68K_Read_Byte_RomF(const u32 addr);
INLINE u8 M68K_Read_Byte_Bad(const u32 addr);
INLINE u8 M68K_Read_Byte_Misc(const u32 addr);
INLINE u8 M68K_Read_Byte_VDP(const u32 addr);
INLINE u8 M68K_Read_Byte_Ram(const u32 addr);

/* Read word */
INLINE u16 M68K_Read_Word_Rom0(const u32 addr);
INLINE u16 M68K_Read_Word_Rom1(const u32 addr);
INLINE u16 M68K_Read_Word_Rom2(const u32 addr);
INLINE u16 M68K_Read_Word_Rom3(const u32 addr);
INLINE u16 M68K_Read_Word_Rom4(const u32 addr);
INLINE u16 M68K_Read_Word_Rom5(const u32 addr);
INLINE u16 M68K_Read_Word_Rom6(const u32 addr);
INLINE u16 M68K_Read_Word_Rom7(const u32 addr);
INLINE u16 M68K_Read_Word_Rom8(const u32 addr);
INLINE u16 M68K_Read_Word_Rom9(const u32 addr);
INLINE u16 M68K_Read_Word_RomA(const u32 addr);
INLINE u16 M68K_Read_Word_RomB(const u32 addr);
INLINE u16 M68K_Read_Word_RomC(const u32 addr);
INLINE u16 M68K_Read_Word_RomD(const u32 addr);
INLINE u16 M68K_Read_Word_RomE(const u32 addr);
INLINE u16 M68K_Read_Word_RomF(const u32 addr);
INLINE u16 M68K_Read_Word_Bad(const u32 addr);
INLINE u16 M68K_Read_Word_Ram(const u32 addr);
INLINE u16 M68K_Read_Word_Misc(const u32 addr);
INLINE u16 M68K_Read_Word_VDP(const u32 addr);


/* Writes */
INLINE void M68K_Write_Bad(const u32 addr, const u8 data);


/* Write byte */
INLINE void M68K_Write_Byte_Bad(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_SRAM(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_Ram(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_Misc(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_VDP(const u32 addr, const u8 data);


/* Write word */
INLINE void M68K_Write_Word_Bad(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_SRAM(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_Ram(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_Misc(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_VDP(const u32 addr, const u16 data);

/* Sega CD */
INLINE u8 M68K_Read_Byte_Bios_CD(const u32 addr);
INLINE u8 M68K_Read_Byte_WRam(const u32 addr);
INLINE u8 M68K_Read_Byte_BRAM_L(const u32 addr);
INLINE u8 M68K_Read_Byte_BRAM(const u32 addr);
INLINE u8 M68K_Read_Byte_BRAM_W(const u32 addr);
INLINE u8 M68K_Read_Byte_Misc_CD(const u32 addr);
INLINE void M68K_Write_Byte_Bios_CD(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_WRam(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_BRAM(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_BRAM_W(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_Misc_CD(const u32 addr, const u8 data);

INLINE u16 M68K_Read_Word_Bios_CD(const u32 addr);
INLINE u16 M68K_Read_Word_WRam(const u32 addr);
INLINE u16 M68K_Read_Word_BRAM_L(const u32 addr);
INLINE u16 M68K_Read_Word_BRAM(const u32 addr);
INLINE u16 M68K_Read_Word_BRAM_W(const u32 addr);
INLINE u16 M68K_Read_Word_Misc_CD(const u32 addr);
INLINE void M68K_Write_Byte_Bios_CD(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_WRam(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_BRAM(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_BRAM_W(const u32 addr, const u8 data);
INLINE void M68K_Write_Byte_Misc_CD(const u32 addr, const u8 data);
INLINE void M68K_Write_Word_Bios_CD(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_WRam(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_BRAM(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_BRAM_W(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_Misc_CD(const u32 addr, const u16 data);


/* 32 X */
INLINE u8 M68K_Read_Byte_Bios_32X(const u32 addr);
INLINE u8 M68K_Read_Byte_BiosR_32X(const u32 addr);
INLINE u8 M68K_Read_Byte_32X_FB0(const u32 addr);
INLINE u8 M68K_Read_Byte_32X_FB1(const u32 addr);
INLINE u8 M68K_Read_Byte_Misc_32X(const u32 addr);

INLINE u16 M68K_Read_Word_Bios_32X(const u32 addr);
INLINE u16 M68K_Read_Word_BiosR_32X(const u32 addr);
INLINE u16 M68K_Read_Word_32X_FB0(const u32 addr);
INLINE u16 M68K_Read_Word_32X_FB1(const u32 addr);
INLINE u16 M68K_Read_Word_Misc_32X(const u32 addr);


INLINE void M68K_Write_Byte_32X_FB0(const u32 addr, const u16 data);
INLINE void M68K_Write_Byte_32X_FB1(const u32 addr, const u16 data);
INLINE void M68K_Write_Byte_Misc_32X(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_32X_FB0(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_32X_FB1(const u32 addr, const u16 data);
INLINE void M68K_Write_Word_Misc_32X(const u32 addr, const u16 data);



/* Memory layout definitions */

u32 Genesis_M68K_Read_Byte_Table[0x20] =
{
	(u32) M68K_Read_Byte_Rom0,		// 0x000000 - 0x07FFFF
	(u32) M68K_Read_Byte_Rom1,		// 0x080000 - 0x0FFFFF
	(u32) M68K_Read_Byte_Rom2,		// 0x100000 - 0x17FFFF
	(u32) M68K_Read_Byte_Rom3,		// 0x180000 - 0x1FFFFF
	(u32) M68K_Read_Byte_Rom4,		// 0x200000 - 0x27FFFF
	(u32) M68K_Read_Byte_Rom5,		// 0x280000 - 0x2FFFFF
	(u32) M68K_Read_Byte_Rom6,		// 0x300000 - 0x37FFFF
	(u32) M68K_Read_Byte_Rom7,		// 0x380000 - 0x3FFFFF
	(u32) M68K_Read_Byte_Rom8,		// 0x400000 - 0x47FFFF
	(u32) M68K_Read_Byte_Rom9,		// 0x480000 - 0x4FFFFF
	(u32) M68K_Read_Byte_RomA,		// 0x500000 - 0x57FFFF
	(u32) M68K_Read_Byte_RomB,		// 0x580000 - 0x5FFFFF
	(u32) M68K_Read_Byte_RomC,		// 0x600000 - 0x67FFFF
	(u32) M68K_Read_Byte_RomD,		// 0x680000 - 0x6FFFFF
	(u32) M68K_Read_Byte_RomE,		// 0x700000 - 0x77FFFF
	(u32) M68K_Read_Byte_RomF,		// 0x780000 - 0x7FFFFF
	(u32) M68K_Read_Byte_Bad,		// 0x800000 - 0x87FFFF
	(u32) M68K_Read_Byte_Bad,		// 0x880000 - 0x8FFFFF
	(u32) M68K_Read_Byte_Bad,		// 0x900000 - 0x97FFFF
	(u32) M68K_Read_Byte_Bad,		// 0x980000 - 0x9FFFFF
	(u32) M68K_Read_Byte_Misc,		// 0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Byte_Bad,		// 0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Byte_VDP,		// 0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Byte_Bad,		// 0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Byte_Ram,		// 0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Byte_Ram,		// 0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Byte_Ram,		// 0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Byte_Ram,		// 0xF80000 - 0xFFFFFF
};


u32 Genesis_M68K_Read_Word_Table[0x20] =
{
	(u32) M68K_Read_Word_Rom0,		// 0x000000 - 0x07FFFF
	(u32) M68K_Read_Word_Rom1,		// 0x080000 - 0x0FFFFF
	(u32) M68K_Read_Word_Rom2,		// 0x100000 - 0x17FFFF
	(u32) M68K_Read_Word_Rom3,		// 0x180000 - 0x1FFFFF
	(u32) M68K_Read_Word_Rom4,		// 0x200000 - 0x27FFFF
	(u32) M68K_Read_Word_Rom5,		// 0x280000 - 0x2FFFFF
	(u32) M68K_Read_Word_Rom6,		// 0x300000 - 0x37FFFF
	(u32) M68K_Read_Word_Rom7,		// 0x380000 - 0x3FFFFF
	(u32) M68K_Read_Word_Rom8,		// 0x400000 - 0x47FFFF
	(u32) M68K_Read_Word_Rom9,		// 0x480000 - 0x4FFFFF
	(u32) M68K_Read_Word_RomA,		// 0x500000 - 0x57FFFF
	(u32) M68K_Read_Word_RomB,		// 0x580000 - 0x5FFFFF
	(u32) M68K_Read_Word_RomC,		// 0x600000 - 0x67FFFF
	(u32) M68K_Read_Word_RomD,		// 0x680000 - 0x6FFFFF
	(u32) M68K_Read_Word_RomE,		// 0x700000 - 0x77FFFF
	(u32) M68K_Read_Word_RomF,		// 0x780000 - 0x7FFFFF
	(u32) M68K_Read_Word_Bad,		// 0x800000 - 0x87FFFF
	(u32) M68K_Read_Word_Bad,		// 0x880000 - 0x8FFFFF
	(u32) M68K_Read_Word_Bad,		// 0x900000 - 0x97FFFF
	(u32) M68K_Read_Word_Bad,		// 0x980000 - 0x9FFFFF
	(u32) M68K_Read_Word_Misc,		// 0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Word_Bad,		// 0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Word_VDP,		// 0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Word_Bad,		// 0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Word_Ram,		// 0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Word_Ram,		// 0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Word_Ram,		// 0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Word_Ram,		// 0xF80000 - 0xFFFFFF
};


u32 Genesis_M68K_Write_Byte_Table[0x10] =
{
	(u32) M68K_Write_Byte_SRAM,		// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Byte_Misc,		// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Byte_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xF00000 - 0xFFFFFF
};

u32 Genesis_M68K_Write_Word_Table[0x10] =
{
	(u32) M68K_Write_Word_SRAM,		// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Word_Misc,		// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Word_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xF00000 - 0xFFFFFF
};

u32 M68K_Read_Byte_Table[0x20] = 
{
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad,
	(u32) M68K_Read_Byte_Bad
};

u32 M68K_Read_Word_Table[0x20] = 
{
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad,
	(u32) M68K_Read_Word_Bad
};

u32 M68K_Write_Byte_Table[0x10] = 
{
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad
};

u32 M68K_Write_Word_Table[0x10] = 
{
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad,
	(u32) M68K_Write_Bad
};


u32 SegaCD_M68K_Read_Byte_Table[0x20] =
{
	(u32) M68K_Read_Byte_Bios_CD,	//  0x000000 - 0x07FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x080000 - 0x0FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x100000 - 0x17FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x180000 - 0x1FFFFF
	(u32) M68K_Read_Byte_WRam,		//  0x200000 - 0x27FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x280000 - 0x2FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x300000 - 0x37FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x380000 - 0x3FFFFF
	(u32) M68K_Read_Byte_BRAM_L,	//  0x400000 - 0x47FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x480000 - 0x4FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x500000 - 0x57FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x580000 - 0x5FFFFF
	(u32) M68K_Read_Byte_BRAM,		//  0x600000 - 0x67FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x680000 - 0x6FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x700000 - 0x77FFFF
	(u32) M68K_Read_Byte_BRAM_W,	//  0x780000 - 0x7FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x800000 - 0x87FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x880000 - 0x8FFFFF
	(u32) M68K_Read_Byte_Bad,		//  0x900000 - 0x97FFFF
	(u32) M68K_Read_Byte_Bad,		//  0x980000 - 0x9FFFFF
	(u32) M68K_Read_Byte_Misc_CD,	//  0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Byte_Bad,		//  0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Byte_Bad,		//  0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Byte_Bad,		//  0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Byte_VDP,		//  0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Byte_Bad,		//  0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Byte_Bad,		//  0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Byte_Bad,		//  0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Byte_Ram,		//  0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Byte_Ram,		//  0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Byte_Ram,		//  0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Byte_Ram,		//  0xF80000 - 0xFFFFFF
};

u32 SegaCD_M68K_Read_Word_Table[0x20] =
{
	(u32) M68K_Read_Word_Bios_CD,	//  0x000000 - 0x07FFFF
	(u32) M68K_Read_Word_Bad,		//  0x080000 - 0x0FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x100000 - 0x17FFFF
	(u32) M68K_Read_Word_Bad,		//  0x180000 - 0x1FFFFF
	(u32) M68K_Read_Word_WRam,		//  0x200000 - 0x27FFFF
	(u32) M68K_Read_Word_Bad,		//  0x280000 - 0x2FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x300000 - 0x37FFFF
	(u32) M68K_Read_Word_Bad,		//  0x380000 - 0x3FFFFF
	(u32) M68K_Read_Word_BRAM_L,	//  0x400000 - 0x47FFFF
	(u32) M68K_Read_Word_Bad,		//  0x480000 - 0x4FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x500000 - 0x57FFFF
	(u32) M68K_Read_Word_Bad,		//  0x580000 - 0x5FFFFF
	(u32) M68K_Read_Word_BRAM,		//  0x600000 - 0x67FFFF
	(u32) M68K_Read_Word_Bad,		//  0x680000 - 0x6FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x700000 - 0x77FFFF
	(u32) M68K_Read_Word_BRAM_W,	//  0x780000 - 0x7FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x800000 - 0x87FFFF
	(u32) M68K_Read_Word_Bad,		//  0x880000 - 0x8FFFFF
	(u32) M68K_Read_Word_Bad,		//  0x900000 - 0x97FFFF
	(u32) M68K_Read_Word_Bad,		//  0x980000 - 0x9FFFFF
	(u32) M68K_Read_Word_Misc_CD,	//  0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Word_Bad,		//  0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Word_Bad,		//  0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Word_Bad,		//  0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Word_VDP,		//  0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Word_Bad,		//  0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Word_Bad,		//  0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Word_Bad,		//  0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Word_Ram,		//  0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Word_Ram,		//  0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Word_Ram,		//  0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Word_Ram,		//  0xF80000 - 0xFFFFFF
};

u32 SegaCD_M68K_Write_Byte_Table[0x10] =
{
	(u32) M68K_Write_Byte_Bios_CD,	// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Bad,			// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Byte_WRam,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Bad,			// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Bad,			// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Bad,			// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Byte_BRAM,		// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Byte_BRAM_W,	// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Bad,			// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Bad,			// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Byte_Misc_CD,	// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Byte_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xF00000 - 0xFFFFFF
};

u32 SegaCD_M68K_Write_Word_Table[0x10] =
{
	(u32) M68K_Write_Word_Bios_CD,	// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Bad,			// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Word_WRam,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Bad,			// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Bad,			// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Bad,			// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Word_BRAM,		// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Word_BRAM_W,	// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Bad,			// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Bad,			// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Word_Misc_CD,	// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Word_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xF00000 - 0xFFFFFF
};

u32 _32X_M68K_Read_Byte_Table[0x20] =
{
	(u32) M68K_Read_Byte_Rom0,		// 0x000000 - 0x07FFFF
	(u32) M68K_Read_Byte_Rom1,		// 0x080000 - 0x0FFFFF
	(u32) M68K_Read_Byte_Rom2,		// 0x100000 - 0x17FFFF
	(u32) M68K_Read_Byte_Rom3,		// 0x180000 - 0x1FFFFF
	(u32) M68K_Read_Byte_Rom4,		// 0x200000 - 0x27FFFF
	(u32) M68K_Read_Byte_Rom5,		// 0x280000 - 0x2FFFFF
	(u32) M68K_Read_Byte_Rom6,		// 0x300000 - 0x37FFFF
	(u32) M68K_Read_Byte_Rom7,		// 0x380000 - 0x3FFFFF
	(u32) M68K_Read_Byte_Bios_32X,	// 0x400000 - 0x47FFFF
	(u32) M68K_Read_Byte_BiosR_32X,	// 0x480000 - 0x4FFFFF
	(u32) M68K_Read_Byte_Bad,		// 0x500000 - 0x57FFFF
	(u32) M68K_Read_Byte_Bad,		// 0x580000 - 0x5FFFFF
	(u32) M68K_Read_Byte_Bad,		// 0x600000 - 0x67FFFF
	(u32) M68K_Read_Byte_Bad,		// 0x680000 - 0x6FFFFF
	(u32) M68K_Read_Byte_Bad,		// 0x700000 - 0x77FFFF
	(u32) M68K_Read_Byte_32X_FB0,	// 0x780000 - 0x7FFFFF
	(u32) M68K_Read_Byte_32X_FB1,	// 0x800000 - 0x87FFFF
	(u32) M68K_Read_Byte_Rom0,		// 0x880000 - 0x8FFFFF
	(u32) M68K_Read_Byte_Rom1,		// 0x900000 - 0x97FFFF
	(u32) M68K_Read_Byte_Rom2,		// 0x980000 - 0x9FFFFF
	(u32) M68K_Read_Byte_Misc_32X,	// 0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Byte_Bad,		// 0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Byte_VDP,		// 0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Byte_Bad,		// 0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Byte_Bad,		// 0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Byte_Ram,		// 0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Byte_Ram,		// 0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Byte_Ram,		// 0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Byte_Ram,		// 0xF80000 - 0xFFFFFF
};

u32 _32X_M68K_Read_Word_Table[0x20] =
{
	(u32) M68K_Read_Word_Rom0,		// 0x000000 - 0x07FFFF
	(u32) M68K_Read_Word_Rom1,		// 0x080000 - 0x0FFFFF
	(u32) M68K_Read_Word_Rom2,		// 0x100000 - 0x17FFFF
	(u32) M68K_Read_Word_Rom3,		// 0x180000 - 0x1FFFFF
	(u32) M68K_Read_Word_Rom4,		// 0x200000 - 0x27FFFF
	(u32) M68K_Read_Word_Rom5,		// 0x280000 - 0x2FFFFF
	(u32) M68K_Read_Word_Rom6,		// 0x300000 - 0x37FFFF
	(u32) M68K_Read_Word_Rom7,		// 0x380000 - 0x3FFFFF
	(u32) M68K_Read_Word_Bios_32X,	// 0x480000 - 0x4FFFFF
	(u32) M68K_Read_Word_BiosR_32X,	// 0x480000 - 0x4FFFFF
	(u32) M68K_Read_Word_Bad,		// 0x500000 - 0x57FFFF
	(u32) M68K_Read_Word_Bad,		// 0x580000 - 0x5FFFFF
	(u32) M68K_Read_Word_Bad,		// 0x600000 - 0x67FFFF
	(u32) M68K_Read_Word_Bad,		// 0x680000 - 0x6FFFFF
	(u32) M68K_Read_Word_Bad,		// 0x700000 - 0x77FFFF
	(u32) M68K_Read_Word_32X_FB0,	// 0x780000 - 0x7FFFFF
	(u32) M68K_Read_Word_32X_FB1,	// 0x800000 - 0x87FFFF
	(u32) M68K_Read_Word_Rom0,		// 0x880000 - 0x8FFFFF
	(u32) M68K_Read_Word_Rom1,		// 0x900000 - 0x97FFFF
	(u32) M68K_Read_Word_Rom2,		// 0x980000 - 0x9FFFFF
	(u32) M68K_Read_Word_Misc_32X,	// 0xA00000 - 0xA7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xA80000 - 0xAFFFFF
	(u32) M68K_Read_Word_Bad,		// 0xB00000 - 0xB7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xB80000 - 0xBFFFFF
	(u32) M68K_Read_Word_VDP,		// 0xC00000 - 0xC7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xC80000 - 0xCFFFFF
	(u32) M68K_Read_Word_Bad,		// 0xD00000 - 0xD7FFFF
	(u32) M68K_Read_Word_Bad,		// 0xD80000 - 0xDFFFFF
	(u32) M68K_Read_Word_Ram,		// 0xE00000 - 0xE7FFFF
	(u32) M68K_Read_Word_Ram,		// 0xE80000 - 0xEFFFFF
	(u32) M68K_Read_Word_Ram,		// 0xF00000 - 0xF7FFFF
	(u32) M68K_Read_Word_Ram,		// 0xF80000 - 0xFFFFFF
};

u32 _32X_M68K_Write_Byte_Table[0x10] =
{
	(u32) M68K_Write_Bad,			// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Bad,			// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Byte_SRAM,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Bad,			// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Bad,			// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Bad,			// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Bad,			// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Byte_32X_FB0,	// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Byte_32X_FB1,	// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Bad,			// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Byte_Misc_32X,	// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Byte_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Byte_Ram,		// 0xF00000 - 0xFFFFFF
};


u32 _32X_M68K_Write_Word_Table[0x10] =
{
	(u32) M68K_Write_Bad,			// 0x000000 - 0x0FFFFF
	(u32) M68K_Write_Bad,			// 0x100000 - 0x1FFFFF
	(u32) M68K_Write_Word_SRAM,		// 0x200000 - 0x2FFFFF
	(u32) M68K_Write_Bad,			// 0x300000 - 0x3FFFFF
	(u32) M68K_Write_Bad,			// 0x400000 - 0x4FFFFF
	(u32) M68K_Write_Bad,			// 0x500000 - 0x5FFFFF
	(u32) M68K_Write_Bad,			// 0x600000 - 0x6FFFFF
	(u32) M68K_Write_Word_32X_FB0,	// 0x700000 - 0x7FFFFF
	(u32) M68K_Write_Word_32X_FB1,	// 0x800000 - 0x8FFFFF
	(u32) M68K_Write_Bad,			// 0x900000 - 0x9FFFFF
	(u32) M68K_Write_Word_Misc_32X,	// 0xA00000 - 0xAFFFFF
	(u32) M68K_Write_Bad,			// 0xB00000 - 0xBFFFFF
	(u32) M68K_Write_Word_VDP,		// 0xC00000 - 0xCFFFFF
	(u32) M68K_Write_Bad,			// 0xD00000 - 0xDFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xE00000 - 0xEFFFFF
	(u32) M68K_Write_Word_Ram,		// 0xF00000 - 0xFFFFFF
};


int CPU_Mode;
int Game_Mode;
int Gen_Version;
int Z80_M68K_Cycle_Tab[CycleTabSize];
u8 Ram_68k[64 * 1024];
u8 *Rom_Data;
//~ u8 Ram_Backup_Ex[64 * 1024];
u8 Genesis_Rom[2 * 1024];
u8 _32X_Genesis_Rom[256];
u8 SRAM[64 * 1024];
u8 OrigSRAM[64 * 1024];
u32 Rom_Size;

int BRAM_Ex_State;
int BRAM_Ex_Size;

int SRAM_Start;
int SRAM_End;
int SRAM_ON;
int SRAM_Write;
int SRAM_Custom;

int S68K_State;
int Z80_State;
int Last_BUS_REQ_Cnt;
int Last_BUS_REQ_St;
int Bank_M68K;
int Bank_SH2;

//~ int CPL_M68K;
//~ int CPL_S68K;
//~ int CPL_Z80;
//~ int Cycles_M68K;
//~ int Cycles_S68K;
//~ int Cycles_Z80;

struct CPUCycles CycleData;

u8 Fake_Fetch;

/*
extern int Controller_1_Counter;
extern int Controller_1_Delay;
extern int Controller_1_State;
extern int Controller_1_COM;
extern int Controller_2_Counter;
extern int Controller_2_Delay;
extern int Controller_2_State;
extern int Controller_2_COM;
//extern Memory_Control_Status;
*/
extern u16 Cell_Conv_Tab[64 * 1024];
//~ extern int VDP_Current_Line;

#ifndef RAZE
extern Z80_CONTEXT M_Z80;
#endif

void Init_Memory_M68K(int System_ID)
{
	int i;

	switch(System_ID)
	{
		case GENESIS:
			for (i=0;i<32;i++)
			{
				// Inicializar manejadores de lectura
				M68K_Read_Byte_Table[i] = Genesis_M68K_Read_Byte_Table[i];
				M68K_Read_Word_Table[i] = Genesis_M68K_Read_Word_Table[i];
			}
			for (i=0;i<16;i++)
			{
				// Inicializar manejadores de escritura
				M68K_Write_Byte_Table[i] = Genesis_M68K_Write_Byte_Table[i];
				M68K_Write_Word_Table[i] = Genesis_M68K_Write_Word_Table[i];
			}
			break;
		case SEGACD:	// TODO
			break;
		case S32X:		// TODO
			break;
	}
}

/*****************************************************************************/
/* Helper memory functions                                                   */
/*****************************************************************************/

/* Read byte */

INLINE u8 M68K_Read_Byte_Bad(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_Rom0(const u32 addr)
{
	u8 val = Rom_Data[(addr & 0x7FFFF) ^ 1];
	return val;
}

INLINE u8 M68K_Read_Byte_Rom1(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x080001];
}

INLINE u8 M68K_Read_Byte_Rom2(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x100001];
}

INLINE u8 M68K_Read_Byte_Rom3(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x180001];
}

INLINE u8 M68K_Read_Byte_Rom4(const u32 addr)
{
	if (SRAM_ON & 1)
	{
		if ((addr >= SRAM_Start) && (addr <= SRAM_End))
		{
			if (SRAM_Custom & 1)
			{
				/* Custom SRAM */
				return 0;
			}
			else
			{
				/* Custom SRAM */
				return SRAM[addr - SRAM_Start];
			}
		}
	}

	return Rom_Data[(addr & 0x7FFFF) ^ 0x200001];
}

INLINE u8 M68K_Read_Byte_Rom5(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x280001];
}

INLINE u8 M68K_Read_Byte_Rom6(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x300001];	
}

INLINE u8 M68K_Read_Byte_Rom7(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x380001];	
}

INLINE u8 M68K_Read_Byte_Rom8(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x400001];	
}

INLINE u8 M68K_Read_Byte_Rom9(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x480001];	
}

INLINE u8 M68K_Read_Byte_RomA(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x500001];	
}

INLINE u8 M68K_Read_Byte_RomB(const u32 addr)
{
	return Rom_Data[(addr & 0x7FFFF) ^ 0x580001];	
}

INLINE u8 M68K_Read_Byte_RomC(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_RomD(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_RomE(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_RomF(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_Ram(const u32 addr)
{
	return Ram_68k[(addr & 0xFFFF) ^ 1];
}


INLINE u8 get_genesis_specs()
{
	return ((((Game_Mode << 1) | CPU_Mode) << 6) | Gen_Version);
}


INLINE u8 M68K_Read_Byte_Misc(const u32 addr)
{
	u8 val=0;

	if (addr > 0xA0FFFF)
	{
		/* No Z80 memory */
		if (addr == 0xA11100)
		{
			if (Z80_State & 2)
			{
				/* Z80 on */
				val = 0x81;
//				return val;
			}
			else
			{
				/* Z80 off */
				if ((M68K_GetCyclesDone() - Last_BUS_REQ_Cnt) > CYCLE_FOR_TAKE_Z80_BUS_GENESIS)
				{
					/* bus taken */
					val = 0x80;
//					return val;
				}
				else
				{
					val = (Last_BUS_REQ_St | 0x80);
//					return val;
				}
			}
		}
		else
		{
			/* No bus request */
			if (addr > 0xA1000D)
			{
				/* wrong access */
				val = WRONG_ACCESS_DATA;
//				return val;
			}
			else
			{
				switch((addr & 0x00000E) >> 1)
				{
					case 0:    /* Genesis specifications */
						val = get_genesis_specs();
						break;

					case 1:    /* Controller 1 */
						val = RD_Controller_1();
						break;

					case 2:    /* Controller 2 */
						val = RD_Controller_2();
						break;

					case 3:
						val = 0xFF;
						break;

					case 4:    /* Controller 1 COM */
						val = Controller_1_COM;
						break;

					case 5:    /* Controller 2 COM */
						val = Controller_2_COM;
						break;

					case 6:
						val = 0;
						break;

					case 7:
						val = WRONG_ACCESS_DATA;
						break;
				}

//				return val;	
			}
		}
	}
	else
	{
		if (Z80_State & 6)
		{
			/* wrong access */
			val = WRONG_ACCESS_DATA;
//			return val;

		}
		else
		{
			/* Z80 read */
			val = Z80_ReadB(addr & 0xFFFF);
			
			//Ristar hack
			extern int Ristar_Hack;
			if (Ristar_Hack && addr == 0xa01fff) {
				val = 0;
			}
//			return val;
		}
	}
	return val;
}

INLINE u8 M68K_Read_Byte_VDP(const u32 addr)
{
	u8 val;

	if ((addr < 0xC00004) || (addr > 0xC00009))
	{
		/* wrong access */
		val = WRONG_ACCESS_DATA;
		return val;
	}
	else
	{
		if (addr < 0xC00008)
		{
			/* Read VDP status */
			u16 vdp_status = Read_VDP_Status();

			/* Swap status register when even address */
			if (addr & 1)
			{
			}
			else
			{
				vdp_status >>= 8;
			}

			val = vdp_status & 0xFF;
			return val;
		}
		else
		{
			if (addr & 1)
			{
				/* Read H counter */
				val = Read_VDP_H_Counter();
				return val;

			}
			else
			{
				/* Read V counter */
				val = Read_VDP_V_Counter();
				return val;
			}
		}
	}
}


/* Read word */

INLINE u16 M68K_Read_Word_Bad(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Rom0(const u32 addr)
{
	u16 val = *((u16 *)(&Rom_Data[addr & 0x7FFFE]));
	return val;
}

INLINE u16 M68K_Read_Word_Rom1(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x080000]));
}

INLINE u16 M68K_Read_Word_Rom2(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x100000]));
}

INLINE u16 M68K_Read_Word_Rom3(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x180000]));
}

INLINE u16 M68K_Read_Word_Rom4(const u32 addr)
{
	if (SRAM_ON)
	{
		if ((addr >= SRAM_Start) && (addr <= SRAM_End))
		{
			if (SRAM_Custom & 1)
			{
				/* Custom SRAM */
				return 0;
			}
			else
			{
				u16 ret_val = *((u16 *) (SRAM + (addr - SRAM_Start)));
				return (((ret_val >> 8) & 0xFF) | ((ret_val << 8) & 0xFF00));
			}
		}
	}

	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x200000]));
}

INLINE u16 M68K_Read_Word_Rom5(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x280000]));
}

INLINE u16 M68K_Read_Word_Rom6(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x300000]));
}

INLINE u16 M68K_Read_Word_Rom7(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x380000]));
}

INLINE u16 M68K_Read_Word_Rom8(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x400000]));
}

INLINE u16 M68K_Read_Word_Rom9(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x480000]));
}

INLINE u16 M68K_Read_Word_RomA(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x500000]));
}

INLINE u16 M68K_Read_Word_RomB(const u32 addr)
{
	return *((u16 *)(&Rom_Data[(addr & 0x7FFFE) + 0x580000]));
}

INLINE u16 M68K_Read_Word_RomC(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_RomD(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_RomE(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_RomF(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Ram(const u32 addr)
{
	return *((u16 *)(Ram_68k + (addr & 0xFFFF)));
}

INLINE u16 M68K_Read_Word_Misc(const u32 addr)
{
	u16 val=0;

	if (addr > 0xA0FFFF)
	{
		/* No Z80 RAM */
		if (addr == 0xA11100)
		{
			if (Z80_State & 2)
			{
				// Z80 on
				Fake_Fetch ^= 0xFF;
				val = ((Fake_Fetch & 0xFF) | 0x8100);
//				return val;
			}
			else
			{
				// Z80 off
				if ((M68K_GetCyclesDone() - Last_BUS_REQ_Cnt) > CYCLE_FOR_TAKE_Z80_BUS_GENESIS)
				{
					Fake_Fetch ^= 0xFF;
					// bus taken
					val = ((Fake_Fetch & 0xFF) | 0x8000);
//					return val;
				}
				else
				{
					Fake_Fetch ^= 0xFF;
					val = ((Fake_Fetch & 0xFF) | ((Last_BUS_REQ_St + 0x80) & 0xFF));
//					return val;
				}
			}
		}
		else
		{
			/* No bus request */
			if (addr > 0xA1000D)
			{
				val = WRONG_ACCESS_DATA;
//				return val;
			}
			else
			{
				switch ((addr & 0x00000E) >> 1)
				{
					case 0:    /* Genesis specifications */
						val = get_genesis_specs();
						break;
					case 1:    /* Controller 1 */
						val = RD_Controller_1();
						break;

					case 2:    /* Controller 2 */
						val = RD_Controller_2();
						break;

					case 3:
						val = 0xFF00;
						break;

					case 4:    /* Controller 1 COM */
						val = Controller_1_COM;
						break;

					case 5:    /* Controller 2 COM */
						val = Controller_2_COM;
						break;

					case 6:
						val = 0;
						break;

					case 7:
						val = WRONG_ACCESS_DATA;
						break;
				}
//				return val;
			}
		}	
	}
	else
	{
		if (Z80_State & 6)
		{
			val = WRONG_ACCESS_DATA;
		}
		else
		{
			val = Z80_ReadB(addr & 0xFFFF);
		}
//		return val;
	}
	return val;
}


INLINE u16 M68K_Read_Word_VDP(const u32 addr)
{
	u16 val;

	if (addr > 0xC00003)
	{
		/* No VDP data */
		if (addr > 0xC00007)
		{
			/* No VDP status */
			if (addr > 0xC00009)
			{
				val = WRONG_ACCESS_DATA;
				return val;
			}
			else
			{
				val = ((Read_VDP_V_Counter() << 8) | Read_VDP_H_Counter());
				return val;
			}
		}
		else
		{
			val = Read_VDP_Status();
			return val;
		}
	}
	else
	{
		val = Read_VDP_Data();
		return val;
	}
}

/* Writes */

INLINE void M68K_Write_Bad(const u32 addr, const u8 data)
{
}


/* Write byte */

INLINE void M68K_Write_Byte_Bad(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Byte_SRAM(const u32 addr, const u8 data)
{
	if ((SRAM_ON & 1) && (SRAM_Write & 1))
	{
		if ((addr >= SRAM_Start) && (addr <= SRAM_End))
		{
			SRAM[addr - SRAM_Start] = data;
		}
	}
}


INLINE void M68K_Write_Byte_Ram(const u32 addr, const u8 data)
{
	Ram_68k[(addr & 0xFFFF) ^ 1] = data;
}


INLINE void M68K_Write_Byte_Misc(const u32 addr, const u8 data)
{
	if (addr > 0xA0FFFF)
	{
		/* No Z80 memory */
		if (addr == 0xA11100)
		{
			Controller_1_Counter = 0;
			Controller_1_Delay = 0;
			Controller_2_Counter = 0;
			Controller_2_Delay = 0;

			if (data & 1)
			{
				/* deactivated */
				/* read 68k clock cycles */
				Last_BUS_REQ_Cnt = M68K_GetCyclesDone();
				Last_BUS_REQ_St = (Z80_State & 2) >> 1;

				if (Z80_State & 2)
				{
					Z80_State &= ~2;
					Z80_Process(Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - Last_BUS_REQ_Cnt]);
//					z80_Exec(&M_Z80, Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - Last_BUS_REQ_Cnt]);
					return;
				}
				else
				{
					/* already deactivated */
					return;
				}
			}
			else
			{
				if (Z80_State & 2)
				{
					/* already activated */
					return;
				}
				else
				{
					Z80_State |= 2;
					Z80_Set_Clock(Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - M68K_GetCyclesDone()]);
//					z80_Set_Odo(&M_Z80, Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - M68K_GetCyclesDone()]);
					return;
				}
			}
		}
		else
		{
			/* No bus request */
			if (addr == 0xA11200)
			{
				if (data & 1)
				{
					// No reset
					Z80_State &= ~4;
					return;
				}
				else
				{
#ifdef RAZE
					z80_reset();
#else
					z80_Reset(&M_Z80);
#endif
					Z80_State |= 4;
					YM2612_Reset();
					return;
				}
			}
			else
			{
				/* No Z80 reset */
				if (addr > 0xA1000D)
				{
					/* No controller IO */
					if (addr < 0xA130F1)
					{
						/* Wrong access */
						return;
					}
					else
					{
						if (addr == 0xA130F1)
						{
							SRAM_ON = data & 1;
							SRAM_Write = (~((data & 2) >> 1))&1;
							return;
						}
						else
						{
							/* No SRAM controller */
							if (addr > 0xA130FF)
							{
								/* Wrong access */
								return;
							}
							else
							{
								/* Mapping switch */
								u8 dst_bnk = (addr & 0xF) >> 1;
								u8 src_bnk = data & 0x1F;

								M68K_Read_Byte_Table[dst_bnk] = Genesis_M68K_Read_Byte_Table[src_bnk];
								M68K_Read_Word_Table[dst_bnk] = Genesis_M68K_Read_Word_Table[src_bnk];
								return;
							}
						}
					}
				}
				else
				{
					switch((addr & 0x00000E) >> 1)
					{
						case 0:
							/* Wrong access */
							return;
						case 1:
							WR_Controller_1(data);
							return;
						case 2:
							WR_Controller_2(data);
							return;
						case 3:
							/* Wrong access */
							return;
						case 4:
							Controller_1_COM = data;
							return;
						case 5:
							Controller_2_COM = data;
							return;
						case 6:
						case 7:
							/* Wrong access */
							return;
					}
				}
			}
		}
	}
	else
	{
		if (Z80_State & 6)
		{
			/* Wrong access */
			return;
		}
		else
		{
			/* Z80 memory write */
			Z80_WriteB(addr & 0xFFFF,data);
		}
	}
}


INLINE void M68K_Write_Byte_VDP(const u32 addr, const u8 data)
{
	if (addr > 0xC00003)
	{
		/* No data port */
		if (addr == 0xC00011)
		{
			PSG_Write(data);
		}
		else
		{
			/* Wrong access */
			return;
		}
	}
	else
	{
		Write_Byte_VDP_Data(data);
	}
}


/* Write Word */

INLINE void M68K_Write_Word_Bad(const u32 addr, const u16 data)
{
}


INLINE void M68K_Write_Word_SRAM(const u32 addr, const u16 data)
{
	if (SRAM_ON & 1)
	{
		if (SRAM_Write & 1)
		{
			if ((addr >= SRAM_Start) && (addr <= SRAM_End))
			{
				*((u16 *)(SRAM + (addr - SRAM_Start))) = ((data >> 8) & 0xFF) | ((data << 8) & 0xFF00);
			}
			else
			{
				/* Wrong access */
				return;
			}
		}
		else
		{
			/* Wrong access */
			return;
		}
	}
	else
	{
		/* Wrong access */
		return;
	}
}


INLINE void M68K_Write_Word_Ram(const u32 addr, const u16 data)
{
//	*((u16 *)(&Ram_68k[addr & 0x7FFFE])) = data;
	*((u16 *)(&Ram_68k[addr & 0xFFFE])) = data;
}


INLINE void M68K_Write_Word_Misc(const u32 addr, const u16 data)
{
	if (addr > 0xA0FFFF)
	{
		/* No Z80 RAM */
		if (addr == 0xA11100)
		{
			Controller_1_Counter = 0;
			Controller_2_Counter = 0;
			Controller_1_Delay = 0;
			Controller_2_Delay = 0;

			if (data & 0x0100)
			{
				// deactivated
				Last_BUS_REQ_Cnt = M68K_GetCyclesDone();
				Last_BUS_REQ_St = (Z80_State & 2) >> 1;

				if (Z80_State & 2)
				{
					Z80_State &= ~2;
					int ciclos_z80 = Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - Last_BUS_REQ_Cnt];

					Z80_Process(ciclos_z80);
//					z80_Exec(&M_Z80, ciclos_z80);
					return;
				}
				else
				{
					// already deactivated
					return;
				}
			}
			else
			{
				if (data & 2)
				{
					// already activated
					return;
				}
				else
				{
					Z80_State |= 2;
					/*
					if (((Cycles_M68K - M68K_GetCyclesDone()) & ~(CycleTabSize-1)) != 0) {
						printf("Something with Z80: a: %i    b: %i   c: %i\n", Cycles_M68K, M68K_GetCyclesDone(), Cycles_M68K - M68K_GetCyclesDone());
						fflush(stdout);
					}*/
					
					extern int Sonic_SMPS_Z80_Hack;
					if (0) {
						//Original code
						Z80_Set_Clock(Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - M68K_GetCyclesDone()]);
					} else if (Sonic_SMPS_Z80_Hack) {
						//SOR2 freezing when trying to play track 2?
						//TODO HACK disabling this works the best for smps z80 sonic games?
						;
					} else {
						//TODO HACK to avoid z80 overrun
						Z80_Set_Clock(Cycles_Z80 - Z80_M68K_Cycle_Tab[(Cycles_M68K - M68K_GetCyclesDone()) & (CycleTabSize-1)]);
					}
					
//					z80_Set_Odo(&M_Z80, Cycles_Z80 - Z80_M68K_Cycle_Tab[Cycles_M68K - M68K_GetCyclesDone()]);
					return;
				}
			}
		}
		else
		{
			/* No bus request */
			if (addr == 0xA11200)
			{
				if (data & 0x0100)
				{
					/* No reset */
					Z80_State &= ~4;
					return;
				}
				else
				{
#ifdef RAZE
					z80_reset();
#else
					z80_Reset(&M_Z80);
#endif
					Z80_State |= 4;
					YM2612_Reset();
					return;
				}
			}
			else
			{
				/* No Z80 reset */
				if (addr > 0xA1000D)
				{
					/* No controller IO */
					if (addr < 0xA130F0)
					{
						/* Wrong access */
						return;
					}
					else
					{
						if (addr == 0xA130F0)
						{
							SRAM_ON = data & 1;
							SRAM_Write = !((data & 2) >> 1);
							return;
						}
						else
						{
							/* No SRAM control */
							if (addr > 0xA130FF)
							{
								/* Wrong access */
								return;
							}
							else
							{
								u16 data_aux = (data & 0xFF00) | ((data >> 8) & 0xFF);
								u8 dst_bnk = (addr & 0xF) >> 1;
								u8 src_bnk = data_aux & 0x1F;

								M68K_Read_Byte_Table[dst_bnk] = Genesis_M68K_Read_Byte_Table[src_bnk];
								M68K_Read_Word_Table[dst_bnk] = Genesis_M68K_Read_Word_Table[src_bnk];
								return;
							}
						}
					}
				}
				else
				{
					switch((addr & 0x00000E) >> 1)
					{
						case 0:
							/* Wrong access */
							return;
						case 1:
							WR_Controller_1(data);
							return;
						case 2:
							WR_Controller_2(data);
							return;
						case 3:
							/* Wrong access */
							return;
						case 4:
							Controller_1_COM = data;
							return;
						case 5:
							Controller_2_COM = data;
							return;
						case 6:
						case 7:
							/* Wrong access */
							return;
					}
				}
			}
		}
	}
	else
	{
		if (Z80_State & 6)
		{
			/* Wrong access */
			return;
		}
		else
		{
			/* Z80 memory write */
			Z80_WriteB(addr & 0xFFFF, data & 0xFF);
		}
	}
}


INLINE void M68K_Write_Word_VDP(const u32 addr, const u16 data)
{
	if (addr > 0xC00003)
	{
		/* No data port */
		if (addr > 0xC00007)
		{
			/* Wrong access */
			return;
		}
		else
		{
			Write_VDP_Ctrl(data);
		}
	}
	else
	{
		Write_Word_VDP_Data(data);
	}
}

/*****************************************************************************/
/* TODO: Sega CD                                                             */
/*****************************************************************************/

INLINE u8 M68K_Read_Byte_Bios_CD(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_WRam(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_BRAM_L(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_BRAM(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_BRAM_W(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_Misc_CD(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Bios_CD(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_WRam(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_BRAM_L(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_BRAM(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_BRAM_W(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Misc_CD(const u32 addr)
{
	return 0;
}

INLINE void M68K_Write_Byte_Bios_CD(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Byte_WRam(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Byte_BRAM(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Byte_BRAM_W(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Byte_Misc_CD(const u32 addr, const u8 data)
{
}

INLINE void M68K_Write_Word_Bios_CD(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_WRam(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_BRAM(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_BRAM_W(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_Misc_CD(const u32 addr, const u16 data)
{
}


/*****************************************************************************/
/* TODO: 32X                                                                 */
/*****************************************************************************/

INLINE u8 M68K_Read_Byte_Bios_32X(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_BiosR_32X(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_32X_FB0(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_32X_FB1(const u32 addr)
{
	return 0;
}

INLINE u8 M68K_Read_Byte_Misc_32X(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Bios_32X(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_BiosR_32X(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_32X_FB0(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_32X_FB1(const u32 addr)
{
	return 0;
}

INLINE u16 M68K_Read_Word_Misc_32X(const u32 addr)
{
	return 0;
}

INLINE void M68K_Write_Byte_32X_FB0(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Byte_32X_FB1(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Byte_Misc_32X(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_32X_FB0(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_32X_FB1(const u32 addr, const u16 data)
{
}

INLINE void M68K_Write_Word_Misc_32X(const u32 addr, const u16 data)
{
}

/*******************************************************************************/
/* Interface functions                                                         */
/*******************************************************************************/

u8 M68K_RB(const u32 addr)
{
	u8 val = ((u8(*)(const u32))M68K_Read_Byte_Table[(addr & 0xF80000) >> 19])(addr & 0xFFFFFF);

#ifdef DEBUG_M68K_MEM
	dbgf("RB[0x%08X]=0x%02X\n",addr,val);
#endif

	return val;
}

u16 M68K_RW(const u32 addr)
{
	u16 val = ((u16(*)(const u32))M68K_Read_Word_Table[(addr & 0xF80000) >> 19])(addr & 0xFFFFFF);

#ifdef DEBUG_M68K_MEM
	dbgf("RW[0x%08X]=0x%04X\n",addr,val);
#endif

	return val;
}

void M68K_WB(const u32 addr, const u8 data)
{
#ifdef DEBUG_M68K_MEM
	dbgf("WB[0x%08X]=0x%02X\n",addr,data);
#endif

	((void(*)(const u32, const u8))M68K_Write_Byte_Table[(addr & 0xF00000) >> 20])(addr & 0xFFFFFF, data & 0xFF);
}

void M68K_WW(const u32 addr, const u16 data)
{
#ifdef DEBUG_M68K_MEM
	dbgf("WW[0x%08X]=0x%04X\n",addr,data);
#endif

	((void(*)(const u32, const u16))M68K_Write_Word_Table[(addr & 0xF00000) >> 20])(addr & 0xFFFFFF, data & 0xFFFF);
}
