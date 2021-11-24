/*****************************************************************************/
/* Modulo VDP Input/Output                                                   */
/* Autor: Oscar Orallo Pelaez                                                */
/* Fecha: 20-02-2007                                                         */
/*****************************************************************************/

#include "gens4all.h"
#include "vdp_io.h"
#ifdef FAME
#include "fame/fame.h"
#else
#include "starscream/star_68k.h"
#endif

#include "mem_m68k.h"
#include "cpu_m68k.h"

#ifndef INLINE
	#define INLINE
#endif

#define IN_VRAM  0
#define IN_CRAM	 1
#define IN_VSRAM 2
#define RD_Mode  0
#define WR_Mode  1

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
#define s32 int

#define VDP_SET1_REG     0
#define VDP_SET2_REG     1
#define VDP_SCRA_REG     2
#define VDP_WIN_REG      3
#define VDP_SCRB_REG     4
#define VDP_SPR_REG      5
#define VDP_REG6         6
#define VDP_BGCOLOR_REG  7
#define VDP_REG8         8
#define VDP_REG9         9
#define VDP_HINT_REG    10
#define VDP_SET3_REG    11
#define VDP_SET4_REG    12
#define VDP_HSCR_REG    13
#define VDP_REG14       14
#define VDP_AUTOINC_REG 15
#define VDP_SCRSIZE_REG 16
#define VDP_WINH_REG    17
#define VDP_WINV_REG    18
#define VDP_DMALENL_REG 19
#define VDP_DMALENH_REG 20
#define VDP_SRCL_REG    21
#define VDP_SRCM_REG    22
#define VDP_SRCH_REG    23
#define VDP_STATUS_REG  24

#define VDP_V32_H32   0x00
#define VDP_V32_H64   0x01
#define VDP_V32_HXX   0x02
#define VDP_V32_H128  0x03
#define VDP_V64_H32   0x04
#define VDP_V64_H64   0x05
#define VDP_V64_HXX   0x06
#define VDP_V64_H128  0x07
#define VDP_VXX_H32   0x08
#define VDP_VXX_H64   0x09
#define VDP_VXX_HXX   0x0A
#define VDP_VXX_H128  0x0B
#define VDP_V128_H32  0x0C
#define VDP_V128_H64  0x0D
#define VDP_V128_HXX  0x0E
#define VDP_V128_H128 0x0F

#define VDP_DEBUG

extern unsigned char *Rom_Data; //[6 * 1024 * 1024];
extern unsigned int Rom_Size;
extern u16 Cell_Conv_Tab[64 * 1024];
extern unsigned short MD_Palette[256];
//extern unsigned short MD_Screen[336 * 240];
//extern Sprite_Struct
extern int Sprite_Struct [0x100 * 8];
extern int Sprite_Visible[0x100];
//~ extern int CPL_M68K;
//~ extern int Cycles_M68K;

#ifdef FAME
#if defined(WIN32) || defined(DREAMCAST) || defined(FAMEC)
#define _m68kcontext m68kcontext
#endif
extern M68K_CONTEXT _m68kcontext;
#else
extern struct S68000CONTEXT main68k_context; // Starscream context (for interrupts)
#endif

extern unsigned int Ram_Word_State;

extern u8 Ram_68k[64 * 1024];
#ifdef SEGA_CD
extern unsigned char Ram_Prg[512 * 1024];
extern unsigned char Ram_Word_2M[256 * 1024];
extern unsigned char Ram_Word_1M[256 * 1024];
#endif
extern int Bank_M68K;


u32	CD_Table[] =
{
	0x0005, 0x0009, 0x0000, 0x000A,	// bits 0-2	= Location (0x00:WRONG, 0x01:VRAM, 0x02:CRAM, 0x03:VSRAM)
	0x0007, 0x000B, 0x0000, 0x0000,	// bits 3-4	= Access   (0x00:WRONG, 0x04:READ, 0x08:WRITE)
	0x0006, 0x0000, 0x0000, 0x0000,	// bits 8-11	= DMA MEM TO VRAM (0x0000:NO DMA, 0x0100:MEM TO VRAM, 0x0200: MEM TO CRAM, 0x0300: MEM TO VSRAM)
	0x0000, 0x0000, 0x0000, 0x0000,	// bits 12	= DMA VRAM FILL (0x0000:NO DMA, 0x0400:VRAM FILL)

	0x0005, 0x0009, 0x0000, 0x000A,	// bits 13	= DMA VRAM COPY (0x0000:NO DMA, 0x0800:VRAM COPY)
	0x0007, 0x000B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,

	0x0005, 0x0509, 0x0000, 0x020A,
	0x0007, 0x030B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,

	0x0800, 0x0100, 0x0000, 0x0200,
	0x0000, 0x0300, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000
};

int DMA_Timing_Table[] =
{
	83,  167, 166,  83,
	102, 205, 204, 102,
	8,    16,  15,   8,
	9,    18,  17,   9
};

int Genesis_Started = 0;
#ifdef SEGA_CD
int SegaCD_Started = 0;
#endif
#ifdef SEGA_32X
int _32X_Started = 0;
#endif
int Size_V_Scroll[] = {255, 511, 255, 1023};
int H_Scroll_Mask_Table[] = {0x0000, 0x0007, 0x01F8, 0x01FF};

u8 VRam[64 * 1024];
u8 CRam[256];
u32 VSRam_Over[8+30];	//Was just 8, but "VSRam_Over[28] = 8" below would write past the end of this array
u8 VSRam[256];
u8 H_Counter_Table[512 * 2];



u32 ScrA_Addr;
u32 ScrB_Addr;
u32 Win_Addr;
u32 Spr_Addr;
u32 H_Scroll_Addr;

u32 H_Cell;
u32 H_Win_Mul;
s32 H_Pix;
u32 H_Pix_Begin;

u32 H_Scroll_Mask;
u32 H_Scroll_CMul;
u32 H_Scroll_CMask;
u32 V_Scroll_CMask;
u32 V_Scroll_MMask;

u32 Win_X_Pos;
u32 Win_Y_Pos;

struct Ctrl_VDP_Type Ctrl;

int DMAT_Tmp;
int DMAT_Type;
#if 0
int DMAT_Lenght;
struct Reg_VDP_Type VDP_Reg;
int VDP_Status;
int VDP_Int;
int VDP_Current_Line;
int VDP_Num_Lines;
int VDP_Num_Vis_Lines;
#else
struct VDPStat VDPStats;
#endif
int CRam_Flag;
int VRam_Flag;

struct Reg_VDP_Type __attribute__ ((aligned (32))) Raster_VDPRegs[MAX_REGISTER_RASTERS];
VDP_Sprite __attribute__ ((aligned (32))) Raster_Sprites[MAX_SPRITE_RASTERS][MAX_SPRITES];
RasterData __attribute__ ((aligned (32))) Raster_Data[256];

int Raster_ColorSplit;
int Raster_ColorSplitLine = 1<<17;
int Raster_SpriteSplitLine = 224;
char __attribute__ ((aligned (32))) Raster_Lines[257];
short __attribute__ ((aligned (32))) Raster_BgColor[256*2];
u8 __attribute__ ((aligned (32))) Raster0_CRam[256];
u8 __attribute__ ((aligned (32))) Raster1_CRam[256];

INLINE void vdp_no_dma(void)
{
	Ctrl.DMA = 0;
}

/* Starts the DMA process if needed */
static void vdp_process_dma(u32 vdp_cd_info)
{
	/* Check for DMA transfer */
	if (!(VDP_Reg.Set2 & 0x10)) 
	{
		vdp_no_dma();
		return;
	}

	/* DMA enable */
	if (vdp_cd_info & 0x4)
	{
		if (Ctrl.DMA_Mode == 0x80)
		{
			Ctrl.DMA = vdp_cd_info & 0xFF;
			return;
		}
	}

	if (!(VDP_Reg.DMA_Lenght & 0xFFFF))
	{
		Ctrl.DMA = 0;
		return;
	}

	/* No DMA fill */
	if (Ctrl.DMA_Mode == 0xC0)
	{
		/* VRAM copy */
		VDP_Status |= 0x0002;

		/* Stef DMA code preview */
		{
			u32 src = VDP_Reg.DMA_Address & 0xFFFF;
			u32 dst = Ctrl.Address;
			u32 dst_step = VDP_Reg.Auto_Inc;
			u32 len = VDP_Reg.DMA_Lenght;
			u16 data;

			VDP_Reg.DMA_Lenght = 0;
			DMAT_Lenght = len;
			VRam_Flag = 1;
			DMAT_Type = 0x3;

			while (len--)
			{
				VRam[dst & 0xFFFF] = VRam[src++];
				src &= 0xFFFF;
				dst += dst_step;
			}

			Ctrl.Address = dst & 0xFFFF;
		}
	}
	else
	{
		if (Ctrl.DMA_Mode & 0x80)
		{
			vdp_no_dma();
			return;
		}

		/* Memory to VRAM */
		/* Stef DMA code preview */
		{
			u32 src = VDP_Reg.DMA_Address << 1;
			u32 dst = Ctrl.Address & 0xFFFF;
			u32 dst_step = VDP_Reg.Auto_Inc;
			u32 len = VDP_Reg.DMA_Lenght & 0xFFFF;
			u16 data;
			int i;

			VDP_Status != 0x0002;
			Ctrl.DMA = 0;

			switch(vdp_cd_info & 3)
			{
				case 0x1: /* VRAM  */
					VRam_Flag = 1;
					DMAT_Type = 0;
					
					if (1) {
						//Detect if writing to sprite table here:
						u32 sprstart = Spr_Addr - (u32)VRam;
						u32 sprend = sprstart + (H_Cell == 32 ? 64*4 : 80*4);
						u32 dmaend = dst + len * dst_step;
						if (sprstart < dmaend && dst < sprend) {
							Raster_Lines[VDP_Current_Line] |= RASTER_LINE_SPRLIST_CHANGE;
							Raster_SpriteSplitLine = VDP_Current_Line;
						}
					}

					for(i=0;i<len;i++)
					{
						data = M68K_RW(src);
						src += 2;
						if (dst & 1)
						{
							data = ((data & 0xFF) << 8) | ((data & 0xFF00) >> 8);
						}
						*(u16 *)(&VRam[dst & 0xFFFF]) = data;
						dst += dst_step;
					}
					break;

				case 0x2: /* CRAM */
					CRam_Flag = 1;
					DMAT_Type = 1;
					dst &= 0x7E;

					for(i=0;i<len;i++)
					{
						data = M68K_RW(src);
						src += 2;
						*(u16 *)(&CRam[dst & 0x7E]) = data;
						dst += dst_step;
					}
					Raster_Lines[VDP_Current_Line] |= RASTER_LINE_COLOR_CHANGE;
					if (!Raster_ColorSplit) {
						Raster_ColorSplit = 1;
						Raster_ColorSplitLine = VDP_Current_Line;
						//~ memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
					}
					break;

				case 0x3: /* VSRAM */
					DMAT_Type = 1;
					dst &= 0x7E;

					for(i=0;i<len;i++)
					{
						data = M68K_RW(src);
						src += 2;
						*(u16 *)(&VSRam[dst & 0x7E]) = data;
						dst += dst_step;
					}
					
					//Updating one or both?
					if (dst_step == 2) {
						Raster_Lines[VDP_Current_Line] |= (dst & 0x2) ? RASTER_LINE_VSCROLLB_CHANGE : RASTER_LINE_VSCROLLA_CHANGE;
					} else {
						Raster_Lines[VDP_Current_Line] |= RASTER_LINE_VSCROLLA_CHANGE | RASTER_LINE_VSCROLLB_CHANGE;
					}
					break;
			}

			Ctrl.Address = dst & 0xFFFF;
			
			if (VDP_Reg.DMA_Lenght <= 0)
			{
				VDP_Reg.DMA_Lenght = 0;
				VDP_Status &= 0xFFFD;
			}
			else
			{
				int dma_cycles;

				VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address + VDP_Reg.DMA_Lenght) & 0x7FFFFF;
				DMAT_Lenght = VDP_Reg.DMA_Lenght;
				VDP_Reg.DMA_Lenght = 0;
				dma_cycles = Update_DMA();
				M68K_ReleaseCycles(); //dma_cycles);
			}
		}
	}

	// No DMA
	vdp_no_dma();
}


//
void Reset_VDP(void)
{
	int i;

	// MD screen
	/*for(i=0;i<(336 * 240);i++)
	{
		MD_Screen[i] = 0;
	}*/

	// VRAM
	for(i=0;i<(64 * 1024);i++)
	{
		VRam[i] = 0;
	}

	// CRAM
	for(i=0;i<256;i++)
	{
		CRam[i] = 0;
	}

	// VSRAM
	for(i=0;i<256;i++)
	{
		CRam[i] = 0;
	}

	// MD palette
	for(i=0;i<256;i++)
	{
		MD_Palette[i] = 0;
	}

	// Sprite struct
	for(i=0;i<(0x100 * 8);i++)
	{
		Sprite_Struct[i] = 0;
	}

	// Sprite visible
	for(i=0;i<0x100;i++)
	{
		Sprite_Visible[i] = 0;
	}

	for(i=0;i<23;i++)
		Set_VDP_Reg(i,0);

	VDP_Reg.DMA_Src_Adr_H = 0;
	VDP_Reg.DMA_Lenght = 0;
	VDP_Reg.DMA_Address = 0;

	/* Setting up status register */
	VDP_Status = 0x0200;

	/* Initializing internal VDP variables */
	VDP_Int = 0;
	DMAT_Tmp = 0;
	DMAT_Lenght = 0;
	DMAT_Type = 0;
	Ctrl.Flag = 0;
	Ctrl.Data = 0;
	Ctrl.Write = 0;
	Ctrl.Access = 0;
	Ctrl.Address = 0;
	Ctrl.DMA_Mode = 0;
	Ctrl.DMA = 0;
	CRam_Flag = 1;
	VRam_Flag = 1;

	for(i=0;i<512;i++)
	{
		H_Counter_Table[i*2] = ((i*170)/488) - 0x18;
		H_Counter_Table[(i*2) + 1] = ((i*205)/488) - 0x1C;
	}

	VSRam_Over[28] = 0;	//This reads past the end of the array...
//	VSRam_Over[7] = 0;
}

u32 Update_DMA(void)
{
	int dma_len;
	u32 res;
	int dma_timing;
	int dma_idx;


	// get DMA timing index
	dma_idx = (VDP_Reg.Set4 & 0x01)*4 + (DMAT_Type & 0x3);

	// if not blanking
	if ((VDP_Current_Line < VDP_Num_Vis_Lines) && (VDP_Reg.Set2 & 0x40))
		dma_idx += 8;

	///////////////////
	// blanking
	///////////////////

	// get dma timing
	dma_timing = DMA_Timing_Table[dma_idx];
	DMAT_Lenght -= dma_timing;

	if (DMAT_Lenght > 0)
	{
		// DMA not finished
		res = (DMAT_Type & 2)?0:CPL_M68K;
		return res;
	}
	else
	{
		int dma_length = DMAT_Lenght;

		// DMA finished, clear DMA busy flag
		VDP_Status &= 0xFFFD;
		DMAT_Lenght = 0;

		if (DMAT_Type & 2)
		{
			res = 0;
		}
		else
		{
			res = (CPL_M68K << 16) / dma_timing;
			res = (res * (dma_length + dma_timing)) >> 16;
		}

		return res;
	}
}

u16 Read_VDP_Data(void)
{
	u16 rdata = 0;         /* Read data */

	Ctrl.Flag = 0;

	/* Proceed depending upon access type */
	switch(Ctrl.Access)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:            /* wrong access */

		case 0x4: return 0; /* read access (wrong) */

		case 0x5: /* VRAM read access */
			rdata = *((u16 *)(&VRam[Ctrl.Address & 0xFFFE]));
			break;
		case 0x6: /* CRAM read access */
			rdata = *((u16 *)(&CRam[Ctrl.Address & 0xFE]));
			break;
		case 0x7: /* VSRAM read access */
			rdata = *((u16 *)(&VSRam[Ctrl.Address & 0xFE]));
			break;

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:           /* write access (wrong) */

		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf: return 0;/* read & write access (wrong) */
	}

	/* Increment address with autoincrement register value */
	Ctrl.Address += VDP_Reg.Auto_Inc;
	Ctrl.Address &= 0xFFFF;

	return rdata;
}


u16 Read_VDP_Status(void)
{
	u16 initial_status = VDP_Status;
	VDP_Status = ((VDP_Status ^ 0xFF00) & 0xFF9F);

	if (VDP_Status & 0x0008)
	{
	}
	else
		VDP_Status &= 0xFF1F;

	// in vblank

	if ((VDP_Reg.Set2 & 0x40) == 0)
	{
		initial_status |= 0x8;
	}

	return initial_status;
}

u8 Read_VDP_H_Counter(void)
{
	u32 tbl_idx = (M68K_GetCyclesDone() - Cycles_M68K - CPL_M68K) & 0x1FF;

	if (VDP_Reg.Set4 & 0x81)	// 40 cell mode ?
		tbl_idx++;

	return H_Counter_Table[tbl_idx];
}
#if 1
u8 Read_VDP_V_Counter(void)
{
	u32 tbl_idx = (M68K_GetCyclesDone() - Cycles_M68K - CPL_M68K) & 0x1FF;
	u32 res;
	u8 aux_val, tbl_val;
	u8 cond_low, cond_high;

	if (VDP_Reg.Set4 & 0x81)	// 40 cell mode ?
	{
		tbl_idx++;
		aux_val = 0xA4;
	}
	else
	{
		aux_val = 0x84;
	}

	tbl_val = H_Counter_Table[tbl_idx];

	cond_high = (tbl_val <= 0xE0)?1:0;
	cond_low = (tbl_val >= aux_val)?1:0;
	cond_low &= cond_high;

	res = VDP_Current_Line + cond_low;

	if (VDP_Status & 1)
	{
		// PAL
		if (res >= 0x103)
		{
			res-= 56;
		}
	}
	else
	{
		if (res >= 0xEB)
		{
			res -= 6;
		}
	}

	// No over line XX
	if (VDP_Reg.Set4 & 2)
	{
		// interlaced
		res = ((res & 0x7F) << 1) | ((res & 0x80) >> 7);
	}

	return res & 0xFF;
}
#else
u8 Read_VDP_V_Counter(void)
{
	int l = VDP_Current_Line;
	if (l >= 0xEB)
		{
			l -= 6;
		}
	return l & 0xff;
}
#endif

void Write_Byte_VDP_Data(u8 Data)
{
	Write_Word_VDP_Data((Data << 8) | Data);
}


void Write_Word_VDP_Data(u16 Data)
{
	Ctrl.Flag = 0;

	/* Check for DMA process */
	if (Ctrl.DMA & 0x4)
	{
		/* DMA fill */
		u32 i;
		u32 dma_length = VDP_Reg.DMA_Lenght & 0xFFFF;
		u32 address = (Ctrl.Address & 0xFFFF);
		u8 fill_data = (Data >> 8) & 0xFF;
		u32 inc = VDP_Reg.Auto_Inc;

		VDP_Reg.DMA_Lenght = 0;
		Ctrl.DMA = 0;
		VDP_Status |= 0x0002;
		DMAT_Type = 2;
		VRam_Flag = 1;
		DMAT_Lenght = dma_length;

		VRam[address ^ 1] = Data & 0xFF;

		if (dma_length == 0)
		{
			dma_length = DMAT_Lenght = 0xFFFF;
		}

		for(i=0;i<=dma_length;i++)
		{
			VRam[address] = fill_data;
			address = (address + inc) & 0xFFFF;
		}

		Ctrl.Address = address & 0xFFFF;

		return;
	}

	/* Proceed depending upon access type */
	switch(Ctrl.Access)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3: return; /* wrong access */

		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7: return; /* read access (wrong) */

		case 0x8: return; /* write access (wrong) */

		case 0x9: /* VRAM write access */
			//Detect if writing to sprite table here:
			if (1) {
				u32 sprstart = Spr_Addr - (u32)VRam;
				u32 sprend = sprstart + (H_Cell == 32 ? 64*4 : 80*4);
				if (Ctrl.Address >= sprstart && Ctrl.Address < sprend) {
					Raster_Lines[VDP_Current_Line] |= RASTER_LINE_SPRLIST_CHANGE;
					Raster_SpriteSplitLine = VDP_Current_Line;
				}
			}
			//VRam_Flag = 1;
			if (Ctrl.Address & 1)
			{
				Data = ((Data & 0xFF) << 8) | ((Data & 0xFF00) >> 8);
			}
			((u16 *)VRam)[(Ctrl.Address & 0xFFFF) >> 1] = Data;
			break;

		case 0xA: /* CRAM write access */
			//CRam_Flag = 1;
			((u16 *)CRam)[(Ctrl.Address & 0x7F) >> 1] = Data;
			//Don't bother creating a raster for bg color changes, since we handle those in Do_Genesis_Frame
			if (((Ctrl.Address & 0x7F) >> 1) != VDP_Reg.BG_Color) {
				Raster_Lines[VDP_Current_Line] |= RASTER_LINE_COLOR_CHANGE;
				if (!Raster_ColorSplit) {
					Raster_ColorSplit = 1;
					Raster_ColorSplitLine = VDP_Current_Line;
					//~ memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
				}
			}
			break;

		case 0xB: /* VSRAM write access */
			((u16 *)VSRam)[(Ctrl.Address & 0x7F) >> 1] = Data;
			Raster_Lines[VDP_Current_Line] |= (Ctrl.Address & 0x2) ? RASTER_LINE_VSCROLLB_CHANGE : RASTER_LINE_VSCROLLA_CHANGE;
			break;

		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF: return; /* read & write access (wrong) */

		default: return;
	}

	/* Increment address with autoincrement register value */
	Ctrl.Address = (Ctrl.Address + VDP_Reg.Auto_Inc) & 0xFFFF;
}


int Set_VDP_Reg(int Num_Reg, int val)
{
	/* Check if register index is valid */
	if ((Num_Reg &= 0x1F) > VDP_STATUS_REG)
	{
// CHUI: Maybe the next is important
		return 0;
	}

	val &= 0xFF;
	VDP_Reg.r[Num_Reg] = val;

	/* Doing some pointer caching and other actions */
	switch(Num_Reg)
	{
		case VDP_SET1_REG:
			//~ VDP_Reg.Set1 = val;
			Update_IRQ_Line();
			break;

		case VDP_SET2_REG:
			//~ VDP_Reg.Set2 = val;
			Update_IRQ_Line();
			break;

		case VDP_SET3_REG:
			//~ VDP_Reg.Set3 = val;
			H_Scroll_Mask = H_Scroll_Mask_Table[val & 3];
			V_Scroll_MMask = (val & 4)?0x7E:0;
			//TODO
			//~ Raster_Lines[VDP_Current_Line] |= RASTER_LINE_VSCROLL_CHANGE;
			break;

		case VDP_SET4_REG:
			//~ VDP_Reg.Set4 = val;
			CRam_Flag = 1;
			if (val & 0x81)
			{
				/* Hcell 40 mode */
				H_Cell = 40;
				H_Win_Mul = 6;
				H_Pix = 320;
				H_Pix_Begin = 0;
				Win_Addr = ((VDP_Reg.Pat_Win_Adr & 0x3C) << 10) + (u32)VRam;
				Win_X_Pos = (VDP_Reg.Win_H_Pos & 0x1F) << 1;

				if (Win_X_Pos > 40) Win_X_Pos = 40;

				Spr_Addr = ((VDP_Reg.Spr_Att_Adr & 0x7E) << 9) + (u32)VRam;
			}
			else
			{
				/* Hcell 32 mode */
				H_Cell= 32;
				H_Win_Mul = 5;
				H_Pix = 256;
				H_Pix_Begin = 32;
				Win_Addr = ((VDP_Reg.Pat_Win_Adr & 0x3E) << 10) + (u32)VRam;

				Win_X_Pos = (VDP_Reg.Win_H_Pos & 0x1F) << 1;

				if (Win_X_Pos > 32) Win_X_Pos = 32;

				Spr_Addr = ((VDP_Reg.Spr_Att_Adr & 0x7F) << 9) + (u32)VRam;
			}
			break;

		case VDP_SCRA_REG:
			//~ VDP_Reg.Pat_ScrA_Adr = val;
			ScrA_Addr = ((val & 0x38) << 10) + (u32)VRam;
			Raster_Lines[VDP_Current_Line] |= RASTER_LINE_MAPA_CHANGE;
			break;

		case VDP_WIN_REG:
			//~ VDP_Reg.Pat_Win_Adr = val;
			Win_Addr = ( ( val & (0x3C | ((VDP_Reg.Set4 & 0x1) << 1) ) ) << 10) + (u32)VRam;
			break;

		case VDP_SCRB_REG:
			//~ VDP_Reg.Pat_ScrB_Adr = val;
			ScrB_Addr = ((val & 0x7) << 13) + (u32)VRam;
			Raster_Lines[VDP_Current_Line] |= RASTER_LINE_MAPB_CHANGE;
			break;

		case VDP_SPR_REG:
			//~ VDP_Reg.Spr_Att_Adr = val;
			//~ VRam_Flag |= 0x2;
			Spr_Addr = ((val & (0x7E | (VDP_Reg.Set4 & 0x1)) ) << 9) + (u32)VRam;
			Raster_Lines[VDP_Current_Line] |= RASTER_LINE_SPRLIST_CHANGE;
			Raster_SpriteSplitLine = VDP_Current_Line;
			break;

		case VDP_BGCOLOR_REG:
			//~ CRam_Flag = 1;
			VDP_Reg.BG_Color = val & 0x3F;
			break;

		case VDP_HINT_REG:
			//~ VDP_Reg.H_Int = val;
			break;

		case VDP_HSCR_REG:
			//~ VDP_Reg.H_Scr_Adr = val;
			H_Scroll_Addr = ((val & 0x3F) << 10) + (u32)VRam;
			//TODO
			//~ Raster_Lines[VDP_Current_Line] |= RASTER_LINE_HSCROLL_CHANGE;
			break;

		case VDP_SCRSIZE_REG:
			//~ VDP_Reg.Scr_Size = val;

			switch (( (val & 0x30) | ((val & 0x3)<<2) ) >> 2)
			{
				case VDP_V32_H32:
				case VDP_VXX_H32:
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 31;
					break;
			
				case VDP_V64_H32:
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 63;
					break;
			
				case VDP_V128_H32:
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 127;
					break;
			
				case VDP_V32_H64:
				case VDP_VXX_H64:
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 31;
					break;
			
				case VDP_V64_H64:
				case VDP_V128_H64:
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 63;
					break;
			
				case VDP_V32_HXX:
				case VDP_V64_HXX:
				case VDP_VXX_HXX:
				case VDP_V128_HXX:
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 0;
					break;
			
				case VDP_V32_H128:
					//~ H_Scroll_CMul = 5;
					//~ H_Scroll_CMask = 31;
					//~ V_Scroll_CMask = 127;
					//~ break;
				case VDP_V64_H128:
				case VDP_VXX_H128:
				case VDP_V128_H128:
					H_Scroll_CMul = 7;
					H_Scroll_CMask = 127;
					V_Scroll_CMask = 31;
					break;
			}
			break;

		case VDP_WINH_REG:
			//~ VDP_Reg.Win_H_Pos = val;

			val = (val & 0x1F) << 1;
			val = (val < H_Cell)?val:H_Cell;
			Win_X_Pos = val;
			Raster_Lines[VDP_Current_Line] |= RASTER_LINE_WIN_CHANGE;
			break;

		case VDP_WINV_REG:
			//~ VDP_Reg.Win_V_Pos = val;
			Win_Y_Pos = val & 0x1F;
			Raster_Lines[VDP_Current_Line] |= RASTER_LINE_WIN_CHANGE;
			break;

		case VDP_DMALENL_REG:
			//~ VDP_Reg.DMA_Lenght_L = val;
			VDP_Reg.DMA_Lenght = (VDP_Reg.DMA_Lenght & 0xFF00) | (val & 0xFF);
			break;

		case VDP_DMALENH_REG:
			//~ VDP_Reg.DMA_Lenght_H = val;
			VDP_Reg.DMA_Lenght = (VDP_Reg.DMA_Lenght & 0xFF) | ((val & 0xFF) << 8);
			break;

		case VDP_SRCL_REG:
			//~ VDP_Reg.DMA_Src_Adr_L = val;
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFFFF00) | val;
			break;

		case VDP_SRCM_REG:
			//~ VDP_Reg.DMA_Src_Adr_M = val;
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFF00FF) | (val << 8);
			break;

		case VDP_SRCH_REG:
			//~ VDP_Reg.DMA_Src_Adr_H = val;
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFFFF) | ((val & 0x7F) << 16);
			Ctrl.DMA_Mode = val & 0xC0;
			break;
/*
		case VDP_REG6:
			VDP_Reg.Reg6 = val;
			break;
		case VDP_REG8:
			VDP_Reg.Reg8 = val;
			break;
		case VDP_REG9:
			VDP_Reg.Reg9 = val;
			break;
		case VDP_REG14:
			VDP_Reg.Reg14 = val;
			break;
		case VDP_AUTOINC_REG:
			VDP_Reg.Auto_Inc = val;
			break;*/
	}

	return val;
}

u8 Int_Ack(void)  // function called by the 68k when an interrupt is acknowledged...
{
	if (VDP_Reg.Set2 & 0x20)
	{
		if (VDP_Int & 8)
		{
			VDP_Int &= ~8;

#ifdef FAME
	if (((VDP_Reg.Set1 & 0x10) >> 2) & VDP_Int)
	{
		unsigned char level = (((VDP_Reg.Set1 & 0x10) >> 2) & VDP_Int);
		unsigned char newinterrupts= 1 << level;
#ifdef DEBUG_M68K
		dbgf("Interrupts = %X\n",newinterrupts);
#endif
		_m68kcontext.interrupts[0] = newinterrupts;
		_m68kcontext.interrupts[level] = level + 0x18;
	}
	else
	{
#ifdef DEBUG_M68K
		dbg("Interrupts 0");
#endif
		_m68kcontext.interrupts[0] = 0;
	}
#endif
			return ((VDP_Reg.Set1 & 0x10) >> 2) & VDP_Int;
		}
	}

#ifdef FAME
#ifdef DEBUG_M68K
	dbg("Interrupts 0");
#endif
	_m68kcontext.interrupts[0] = 0;
#endif

	VDP_Int = 0;
	return 0;
}

void Update_IRQ_Line(void)
{
	if (VDP_Reg.Set2 & 0x20)
	{
		if (VDP_Int & 0x8)
		{
			M68K_Interrupt(6);
			return;
		}
	}

	if (VDP_Reg.Set1 & 0x10)
	{
		if (VDP_Int & 0x4)
		{
			M68K_Interrupt(4);
			return;
		}
	}

#ifdef DEBUG_M68K
	dbg("Interrupts 0");
#endif
#ifdef FAME
	_m68kcontext.interrupts[0] = 0;
#else
	*((u8 *)(((u32)(&main68k_context)) + (35 * 4))) &= 0xF0;
#endif
}

int Write_VDP_Ctrl(u16 Data)
{
	u32 vdp_cd_info;
	u32 cd_info_idx;
	static u16 first_word=0, second_word=0;

	if (Ctrl.Flag & 1)
	{
		/* Second write in progress */
		Ctrl.Flag = 0;         /* Write finished */
		second_word = Data;

		Ctrl.Data = (Data << 16) | (Ctrl.Data & 0xFFFF);

		/* Setting up VDP address from data */
		Ctrl.Address = ((Data << 14) | (Ctrl.Data & 0x3FFF)) & 0xFFFF;

		/* Getting CD info and setting up VDP access type */
		cd_info_idx = (((first_word & 0xC000) >> 12) | (Data & 0xF0)) >> 2;

		vdp_cd_info = CD_Table[cd_info_idx];
		Ctrl.Access = vdp_cd_info & 0xFF;

		vdp_cd_info = (vdp_cd_info & 0xFF00) | ((vdp_cd_info >> 8) & 0xFF);

		if (vdp_cd_info & 0xFF00)
		{
			/* Do DMA here */
			vdp_process_dma(vdp_cd_info);
		}
	}
	else
	{
		/* First write... */

		if ((Data & 0xC000) == 0x8000)
		{
			Ctrl.Access = 5;
			Ctrl.Address = 0;
			Set_VDP_Reg((Data >> 8) & 0x1F, Data & 0xFF);
		}
		else
		{
			/* First write */
			Ctrl.Flag = 1;     /* Write in progress */
			first_word = Data;
			Ctrl.Data = (Ctrl.Data & 0xFFFF0000) | Data;
	
			/* Setting up VDP address from data */
			Ctrl.Address = ((((Ctrl.Data & 0xFFFF0000) >> 16) << 14) & 0xC000) | (Data & 0x3FFF);
	
			/* Getting CD info and setting up VDP access type */
			vdp_cd_info = CD_Table[(((Data & 0xC000) >> 12) | ((Ctrl.Data >> 16) & 0xF0)) >> 2];
			Ctrl.Access = vdp_cd_info & 0xFF;
		}
	}
// CHUI: Maybe the next is important
	return 0;
}

