#ifdef DEBUG_VDP

// #define DEBUG_VDP_CHKSUM

#include "gens4all.h"
#include "vdp_rend.h"

#define u8 unsigned char


extern int Sprite_Struct [0x100 * 8];
extern int Sprite_Visible [0x100];
extern unsigned int H_Scroll_Addr;
extern int H_Scroll_CMul;
extern int VDP_Current_Line;
extern u8 VRam[64 * 1024];

extern Data_Misc_Type Data_Misc;


extern Data_Spr_Type Data_Spr;
extern unsigned H_Pix;
extern int VDP_Status;
extern unsigned int H_Scroll_Mask;
extern unsigned int H_Scroll_Addr;
extern unsigned int H_Scroll_CMask;
extern unsigned int H_Cell;

#ifdef DEBUG_EXTRA_VDP
void print_sprite_struct();

void print_spr_vis(void)
{
int i;
static int call=0;

	if (call == 62132)
	{
		dbgf("cur_line: %d\n",VDP_Current_Line);
		dbgf("Spr_End:%d\n",Data_Misc.Spr_End);
	
		for(i=0;i<0x100*8;i+=8)
		{
			dbgf("i=%d\n",i);
			dbgf("SS[1]=%d\n",Sprite_Struct[i + 1]);
			dbgf("SS[5]=%d\n",Sprite_Struct[i + 5]);
		}
	
	print_sprite_struct();
	}


	dbgf("spr_vis_%d:\n",call++);
	dbgf("Borne: %d\n",Data_Misc.Borne);


	for(i=0;i<Data_Misc.Borne/4;i++)
	{
		dbgf("SV%d=%d\n",i,Sprite_Visible[i]);
	}
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_spr_vis)",MD_Screen,336 * 240 *2);
#endif
}

void print_sprite_struct()
{
	int i;

	for(i=0;i<0x100*8;i++)
	{
		dbgf("SS%d=%d\n",i,Sprite_Struct[i]);
	}
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_sprite_struct)",MD_Screen,336 * 240 *2);
#endif
}

void print_pattern_data(int data)
{
	static int i=0;

	dbgf("PD_%d:%d\n",i++,data);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_pattern_data)",MD_Screen,336 * 240 *2);
#endif
}

void print_putline(unsigned dest,unsigned palette_number)
{
	dbgf("PUTLINE %X, %X\n",dest,palette_number);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_putline)",MD_Screen,336 * 240 *2);
#endif
}

void print_pixel8(unsigned addr,unsigned char pix)
{
	dbgf("Pixel %X=%.2X\n",addr,pix);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_pixel8)",MD_Screen,336 * 240 *2);
#endif
}

void print_pattern_info(int data)
{
	static int i=0;

	dbgf("PI_%d:%d\n",i++,data);
//	dbgf("HS%d:%d - VS:%d\n",i++,hscroll,vscroll);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_pattern_info)",MD_Screen,336 * 240 *2);
#endif
}

void print_yoffset(int data)
{
	static int i=0;

	dbgf("YOFF_%d:0x%08X\n",i++,data);
	dbgf("X=0x%08X\n",Data_Misc.X);
	dbgf("C=0x%08X\n",Data_Misc.Cell);
	dbgf("H=0x%08X\n",H_Cell);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_yoffset)",MD_Screen,336 * 240 *2);
#endif
}

void print_xoffset(unsigned int data)
{
#if 0
	static int i=0;

	dbgf("XOFF_%d:0x%04X\n",i++,data&0xFFFF);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_xoffset)",MD_Screen,336 * 240 *2);
#endif
#endif
}

void print_current_line(void)
{
	dbgf("CL: %d\n",VDP_Current_Line);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_current_line)",MD_Screen,336 * 240 *2);
#endif
}

//int ahora=0;
void print_render_line_start(void)
{
	static unsigned long long i=0;
	dbgf("RLS-%.8X%.8X\n",(unsigned)(i>>32),(unsigned)(i&0xFFFFFFFF));
	i++;
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_render_line_start)",MD_Screen,336 * 240 *2);
#endif
/*
{
//static unsigned cuenta=0;
//int back=debuggingnow;
//cuenta++;
//if (cuenta>156369) debuggingnow=ahora=1;
//if (cuenta>156370) exit(0);
	dbgsum("RLS",MD_Screen,336 * 240 *2);
//debuggingnow=back;
}
*/
}

void print_debug_putline_sprite(int dest)
{
	dbgf("dest = %d\n",dest);
	dbgf("HMAX = %d\n", Data_Spr.H_Max);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_debug_putline_sprite)",MD_Screen,336 * 240 *2);
#endif
}

void print_special_debug_line_spr(int dest,int spr_idx)
{
	dbgf("destA = %d\n",dest);
	dbgf("spr_idx = %d\n",spr_idx);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_special_debug_line_spr)",MD_Screen,336 * 240 *2);
#endif
}

void print_putpixel_sprite(int pattern_data, int first_pat)
{
	static int i=0;

	dbgf("PS%d: 0x%08X - 0x%08X - 0x%08X\n",i++,pattern_data,first_pat,VDP_Status);
//	dbgf("HMax=%d\n",Data_Spr.H_Max);
//	dbgf("HPix=%d\n",H_Pix);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_putpixel_sprite)",MD_Screen,336 * 240 *2);
#endif
}

void print_first_pat(int first_pat)
{
	static int i=0;
	dbgf("FP%d: 0x%08X\n",i++,first_pat);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_first_pat)",MD_Screen,336 * 240 *2);
#endif
}

void print_pixel_value(int pixel_value, int palnum)
{
	static int i=0;

	dbgf("PV%d: 0x%02X - 0x%08X\n",i++,pixel_value & 0xFF,palnum);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_pixel_value)",MD_Screen,336 * 240 *2);
#endif
}

void print_cell_info(int cell_info)
{
	dbgf("CI: %d\n",cell_info);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_cell_info)",MD_Screen,336 * 240 *2);
#endif
}


void print_debug_scroll_a(unsigned int dest)
{
#if 0
	dbgf("SA=0x%08X\n",Data_Misc.Start_A);
	dbgf("LA=0x%08X\n",Data_Misc.Lenght_A);
	dbgf("SW=0x%08X\n",Data_Misc.Start_W);
	dbgf("LW=0x%08X\n",Data_Misc.Lenght_W);
	dbgf("CELL=0x%08X\n",Data_Misc.Cell);
	dbgf("Mask=0x%08X\n",Data_Misc.Mask);
	dbgf("CMask=0x%08X\n",H_Scroll_CMask);
	dbgf("Dest=0x%08X\n",dest);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_debug_scroll_a)",MD_Screen,336 * 240 *2);
#endif
#endif
}

void print_tmp_value_win(void)
{
#if 0
	dbgf("SA1=0x%08X\n",Data_Misc.Start_A);
	dbgf("LA1=0x%08X\n",Data_Misc.Lenght_A);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_tmp_value_win)",MD_Screen,336 * 240 *2);
#endif
#endif
}

void print_offsets(int x, int y)
{
#if 0
	dbgf("X=0x%08X\n",x);
	dbgf("Y=0x%08X\n",y);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_offsets)",MD_Screen,336 * 240 *2);
#endif
#endif
}

void print_tag(int x)
{
//int back=debuggingnow;
//if (!debuggingnow) debuggingnow=ahora;
	dbgf("TAG %i\n",x);
#ifdef DEBUG_VDP_CHKSUM
	dbgsum("MD_Screen (print_tag)",MD_Screen,336 * 240 *2);
#endif
//debuggingnow=back;
}

void print_byte(unsigned char value)
{
	dbgf(" %.2X\n",value&0xFF);
}

void print_word(unsigned short value)
{
	dbgf(" %.4X\n",value&0xFFFF);
}

void print_dword(unsigned value)
{
	dbgf(" %.8X\n",value);
}
#else
void print_mdscreen(void)
{
	dbgsum("MD_Screen",MD_Screen,336 * 240 *2);
}
#endif

#endif
