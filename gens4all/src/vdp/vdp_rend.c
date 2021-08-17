/*****************************************************************************/
/* Modulo VDP rendering                                                      */
/*****************************************************************************/

#include "gens4all.h"
#include "vdp_rend.h"
#include "vdp_io.h"
#include "vdp_spy.h"


#define U32 unsigned int
#define U16 unsigned short
#define U8 unsigned char
#define S32 int
#define S16 short
#define S8 char

#define VDP_SPR_DEF_SIZE 8

#define VDP_HIGH_B      0x80
#define VDP_SHADOW_B    0x40
#define VDP_PRIO_B      0x01
#define VDP_SPR_B       0x20

#define VDP_HIGH_W    0x8080
#define VDP_SHADOW_W  0x4040
#define VDP_SHADOW_WH 0x4000
#define VDP_NOSHAD_W  0xBFBF
#define VDP_PRIO_W    0x0100
#define VDP_SPR_W     0x2000

#define VDP_SHADOW_D   0x40404040
#define VDP_NOSHADOW_D 0xBFBFBFBF

#define SCROLL_A 0
#define SCROLL_B 1

#define PALETTE_SIZE 64


/* Small optimization over sprite A rendering */
//#define SPRA_OPT

extern U8 VRam[64 * 1024];
extern S32 VRam_Flag;
extern U8 CRam[256];
extern S32 CRam_Flag;
extern U8 VSRam[256];
extern struct Reg_VDP_Type VDP_Reg;
extern S32 VDP_Current_Line;
extern S32 VDP_Status;
extern S32 H_Cell;
extern U32 H_Win_Mul;
extern S32 H_Pix;
extern U32 H_Pix_Begin;
extern U32 H_Scroll_Mask;
extern U32 H_Scroll_CMul;
extern U32 H_Scroll_CMask;
extern U32 V_Scroll_CMask;
extern U32 V_Scroll_MMask;
extern U32 Win_X_Pos;
extern U32 Win_Y_Pos;
extern U32 ScrA_Addr;
extern U32 ScrB_Addr;
extern U32 Win_Addr;
extern U32 Spr_Addr;
extern U32 H_Scroll_Addr;

#ifdef SPRA_OPT
U32 Mask_N[] = {0xFFFFFFFF, 0xFFF0FFFF, 0xFF00FFFF, 0xF000FFFF, 0x0000FFFF, 0x0000FFF0, 0x0000FF00, 0x0000F000};
U32 Mask_F[] = {0xFFFFFFFF, 0xFFFF0FFF, 0xFFFF00FF, 0xFFFF000F, 0xFFFF0000, 0x0FFF0000, 0x00FF0000, 0x000F0000};
#endif

S32 interlaced;
S32 hishadow;
S32 scroll_mode;
U32 pattern_data;
U16 pattern_info;
U32 palette_number;
U32 vdp_cur_line_pixel;     // VDP_Current_Line * 336
U32 xoffset;

U16 MD_Screen[336 * 240];
U16 MD_Palette[256];
U16 Palette[4096];
S32 Sprite_Struct [0x100 * 8];
S32 Sprite_Visible[0x100];
Data_Spr_Type Data_Spr;
Data_Misc_Type Data_Misc;
S32 Sprite_Over;
S32 Mode_555;

U16 filling_value;	/* Filling colour for the background */
U16 adj_value;		/* Adjusted value for pixel rendering */


INLINE U32 get_xoffset(void)
{
	const U32 *ptr = (U32 *)H_Scroll_Addr;

	return ptr[VDP_Current_Line & H_Scroll_Mask];
}

static void update_yoffset(U32 *yoffset, const int scroll)
{
	if (!(Data_Misc.Cell & 0xFF81))
	{
		const U32 *ptr = (U32 *)VSRam;
		U32 tmp;

		tmp = ptr[Data_Misc.Cell>>1];
		tmp >>= (scroll << 4) + interlaced;
		Data_Misc.Line_7 = (VDP_Current_Line + tmp) & 7;
		*yoffset = ((VDP_Current_Line + tmp) >> 3) & V_Scroll_CMask;
	}
}

INLINE U16 get_pattern_info(const U16 *scroll_addr, const U32 h_cell_offset, const U32 v_offset)
{
// 	const U32 tmp = v_cell_offset << H_Scroll_CMul;	// V Cell Offset * H Width

	return scroll_addr[h_cell_offset + v_offset];
}


INLINE U32 get_pattern_data(void)
{
	const U32 tmp = (pattern_info & 0x1000)?(Data_Misc.Line_7 ^ 7):Data_Misc.Line_7;
	const U32 *ptr_vram = (U32 *)VRam;

	return ptr_vram[ (((pattern_info & 0x7FF) << (5 + interlaced)) >> 2) + (tmp << interlaced) ];
}


/******************************************************************************/
/* Builds the sprite struct                                                   */
/* Sprite definition format:                                                  */
/* [0] = x position                                                           */
/* [1] = y position                                                           */
/* [2] = x size                                                               */
/* [3] = y size - 1                                                           */
/* [4] = x position max                                                       */
/* [5] = y position max                                                       */
/* [6] = first tile index                                                     */
/******************************************************************************/
static void build_spr_struct(S32 interlaced)
{
	S32 i, spr_idx;
	S32 posx, posy, posx_max, posy_max;
	S32 spr_size, xsize;
	S32 first_tile;
	U16 *spr_ptr;
	U8 *spr_ptr8;
	U16 spr_data;
	S8 next_sprite;


	spr_ptr = (U16 *) Spr_Addr;
	spr_ptr8 = (U8 *) Spr_Addr;


	for(spr_idx=0;spr_idx<(80 * 8);spr_idx+=8)
	{
		posy = ((spr_ptr[0]>>interlaced) & 0x1FF) - 0x80;
		posx = (spr_ptr[3] & 0x1FF) - 0x80;
		spr_size = spr_ptr8[2 ^ 1];

		xsize = spr_size;
		xsize &= 0x0C;
		spr_size &= 0x03;
		xsize >>= 2;

		Sprite_Struct[spr_idx + 0] = posx;
		Sprite_Struct[spr_idx + 1] = posy;
		xsize++;
		Sprite_Struct[spr_idx + 2] = xsize & 0xFF;
		Sprite_Struct[spr_idx + 3] = spr_size & 0xFF;

		Sprite_Struct[spr_idx + 4] = posx + (xsize & 7)*8 - 1;
		Sprite_Struct[spr_idx + 5] = posy + (spr_size & 3)*8 + 7;
		Sprite_Struct[spr_idx + 6] = spr_ptr[2];

		next_sprite = spr_ptr8[3 ^ 1] & 0x7F;
//print_byte(next_sprite);	
		if (next_sprite == 0)
		{
			/* Store the last sprite index */
			Data_Misc.Spr_End = spr_idx;
// print_sprite_struct();
			return;
		}

		spr_ptr = (U16 *)(Spr_Addr + (next_sprite * 8));
		spr_ptr8 = (U8 *)spr_ptr;
	}

// print_sprite_struct();

	/* Store the last sprite index */
	Data_Misc.Spr_End = spr_idx-8;
}

/******************************************************************************/
/* Builds the sprite struct partially                                         */
/* Sprite definition format                                                   */
/* [0] = x position                                                           */
/* [1] = y position                                                           */
/* [2] = x size                                                               */
/* [3] = y size - 1                                                           */
/* [4] = x position max                                                       */
/* [5] = y position max                                                       */
/* [6] = first tile index                                                     */
/******************************************************************************/
static void build_spr_struct_partial(void)
{
	S32 spr_idx;
	S32 posx, posx_max;
	U8 xsize;
	S32 first_tile;


	U16 *spr_ptr;
	U8 *spr_ptr8;
	U16 spr_data;
	S16 *spr_struct_ptr = (S16 *)Sprite_Struct;

	U8 spr_size;
	S8 next_spr;

	spr_ptr = (U16 *) Spr_Addr;
	spr_ptr8 = (U8 *) Spr_Addr;

	for(spr_idx=0;spr_idx<(80*8);spr_idx+=8)
	{
		spr_size = spr_ptr8[2 ^ 1] & 0x0C;
		next_spr = spr_ptr8[3 ^ 1] & 0x7F;
		posx = (spr_ptr[3] & 0x1FF) - 0x80;
		first_tile = spr_ptr[2];
		
		spr_struct_ptr[spr_idx + 12] = first_tile;
		Sprite_Struct[spr_idx] = posx;

		posx_max = posx + spr_size*2 + 7;
		Sprite_Struct[spr_idx + 4] = posx_max;

		if (next_spr == 0) break;
		
		spr_ptr = (U16 *) (Spr_Addr + (next_spr*8));
		spr_ptr8 = (U8 *)spr_ptr;
	}
}

/* Updates the palette                  */
/* hi_shadow = highlight/shadow enabled */
static void update_palette(S32 hi_shadow)
{
	S32 pal_idx;
	U16* cram_ptr = (U16 *)CRam;
	S32 mask;
	U16 color;

	CRam_Flag = 0;
	mask = (Mode_555 & 1)?0x3DEF:0x7BEF;

	for(pal_idx = 0; pal_idx < PALETTE_SIZE; pal_idx ++)
	{
		color = Palette[cram_ptr[pal_idx] & 0x0FFF];

		/* Store the normal color */
		MD_Palette[pal_idx] = color;

		/* Shadow/highlight color enable */
		if (hi_shadow)
		{
			/* Store the normal color */
			MD_Palette[pal_idx + 192] = color;

			color >>= 1;

			/* Shadow: divide the color components by 2 */
			color &= mask;

			/* Store the shadow color */
			MD_Palette[pal_idx + 64] = color;

			/* Highlight: add the mask to color components */
			color += mask;

			/* Store the highlight color */
			MD_Palette[pal_idx + 128] = color;
		}
	}

	color = MD_Palette[VDP_Reg.BG_Color];
	MD_Palette[0] = color;

	if(hi_shadow)
	{
		MD_Palette[192] = color;
		color >>= 1;
		color &= mask;
		MD_Palette[64] = color;
		color += mask;
		MD_Palette[128] = color;
	}
}


/* Update sprite mask                         */
/* spr_limit = sprite emulation limit enabled */
INLINE void update_spr_mask(S32 spr_limit)
{
	S32 idx = 0;		// indice sprite struct
	S32 idx_vis = 0;	// indice sprite struct visible
	S32 max_xcells = H_Cell;
	S32 cur_line = VDP_Current_Line;
	S32 hpix = H_Pix;

//print_tag(100);
//print_mdscreen();
//print_dword(Data_Misc.Spr_End);
Loop1:
//print_tag(1);
	if ((Sprite_Struct[idx + 1] > cur_line) || (Sprite_Struct[idx + 5] < cur_line))
	{
//print_tag(2);
		// Out_Line_1
		idx += 8;
		if (idx <= Data_Misc.Spr_End) goto Loop1;
	}
	else
	{
//print_tag(3);
		// sprite limit actived
		if (spr_limit) max_xcells -= Sprite_Struct[idx + 2];

//print_dword(Sprite_Struct[(idx >> 2) + 0]);
//print_dword(hpix);
//print_dword(Sprite_Struct[(idx >> 2) + 4]);
		if ((Sprite_Struct[idx + 0] < hpix) && (Sprite_Struct[idx + 4] >= 0))		
		{
//print_tag(4);
			Sprite_Visible[idx_vis++] = idx;
		}


		// Out_Line_1_2
		idx += 8;

		if (idx <= Data_Misc.Spr_End)
		do
		{
//print_tag(5);
		// Loop2
			if ((Sprite_Struct[idx + 1] <= cur_line) && (Sprite_Struct[idx + 5] >= cur_line))
			{
//print_tag(6);
				// Loop2_First
				if (Sprite_Struct[idx + 0] == -128)
					break;

				// sprite limit actived
				if (spr_limit) max_xcells -= Sprite_Struct[idx + 2];

				if ((Sprite_Struct[idx + 0] < hpix) && (Sprite_Struct[idx + 4] >= 0))
				{
//print_tag(7);
					Sprite_Visible[idx_vis++] = idx;
				}
			}

//print_tag(8);
			// Out_Line_2
			idx += 8;
//print_dword(idx);
			if (spr_limit)
			{
				if (max_xcells <= 0)
				{
//print_tag(9);
					//sprite overflow
					if (idx > Data_Misc.Spr_End)
						break;
	
					// Proceed with VDP_Status modification if needed
					do
					{
						if ((Sprite_Struct[idx + 1] > cur_line) || (Sprite_Struct[idx + 5] < cur_line))
							idx += 8;
						else
						{
							VDP_Status |= 0x40;
							goto End;
						}
					} while (idx <= Data_Misc.Spr_End);
					break;
				}
			}
		} while (idx <= Data_Misc.Spr_End);
	}

End:
	// store the number of visible sprites
	Data_Misc.Borne = idx_vis;
}

INLINE void plotpixel(const U32 dest, const U32 data)
{
	MD_Screen[dest] = data;
}

/* Putpixel sprite */
// Returns scr data to putline_sprite
static void putpixel_spr(const U32 dest, const U32 npixel, U32 pix_data, const S32 priority)
{
	U8 *scr = (U8*)MD_Screen;      /* pointer to the MD screen */
	const U32 scr_idx = dest + npixel + 8;
	U8 scr_data;

	scr_data = scr[(scr_idx*2) + 1];

	if (scr_data & (VDP_PRIO_B + VDP_SPR_B - priority))
	{
//print_tag(3);
		scr[(scr_idx*2) + 1] |= VDP_SPR_B;
		VDP_Status |= VDP_SPR_B;
	}
	else
	{
//print_tag(4);
		// Put pixel

print_pixel_value(scr_data,pix_data);

		pix_data += (palette_number + VDP_SPR_W);

		if (hishadow)
		{
//print_tag(5);
			if (priority < 1)
			{
//print_tag(6);
				scr_data &= (VDP_SHADOW_B | VDP_HIGH_B);
			}
			else
			{
//print_tag(7);
				scr_data &= VDP_HIGH_B;
			}

			if (pix_data < (0x3E + VDP_SPR_W))
			{
//print_tag(8);
				// Normal
				pix_data += scr_data;
			}
			else if (pix_data > (0x3E + VDP_SPR_W))
			{
//print_tag(9);
				// Shadow
				MD_Screen[scr_idx] |= VDP_SHADOW_W;
// CHUI: Maybe the next is important
//print_tag(12);
				return;
			}
			else
			{
//print_tag(10);
				// Highlight
				MD_Screen[scr_idx] |= VDP_HIGH_W;
// CHUI: Maybe the next is important
//print_tag(12);
				return;
			}
		}
		MD_Screen[scr_idx] = pix_data;
//print_tag(11);
	}
}


static void mask_scroll_b_p1(const U32 dest)
{
	if (hishadow)
	{
		S32 i;

		for(i=0;i<8;i++)
			MD_Screen[dest + i] &= VDP_NOSHAD_W;
	}
}

INLINE void revert_pattern(void)
{
	pattern_data = (pattern_data << 28) |
				((pattern_data << 20) & 0x0f000000) |
				((pattern_data << 12) & 0x00f00000) |
				((pattern_data << 4)  & 0x000f0000) |
				((pattern_data >> 4)  & 0x0000f000) |
				((pattern_data >> 12) & 0x00000f00) |
				((pattern_data >> 20) & 0x000000f0) |
				((pattern_data >> 28) & 0x0000000f);
}


INLINE void process_pattern_scra_win(const U32 dest, const U32 pos)
{
	/* Test to see if the pixel is transparent */
	if (pattern_data & 0xf)
		plotpixel(dest + pos, (pattern_data & 0xf) + adj_value);

	pattern_data >>= 4;
}

INLINE void process_pattern_scra_win_p0(const U32 dest, const U32 pos)
{
	/* Test to see if the pixel is transparent */
	if (pattern_data & 0xf)
	{
		const U16 tmp = MD_Screen[dest + pos];

		if (tmp & VDP_PRIO_W) return;

		plotpixel(dest + pos, (pattern_data & 0xf) + (tmp & VDP_SHADOW_WH) + adj_value);
	}

	pattern_data >>= 4;
}


INLINE void process_pattern(const U32 dest, const U32 pos)
{
	/* Test to see if the pixel is transparent */
	if (pattern_data & 0xf)
		plotpixel(dest + pos, (pattern_data & 0xf) + adj_value);
	else
		plotpixel(dest + pos, filling_value);

	pattern_data >>= 4;
}

static void putline(const U32 dest)
{
	if (pattern_data)
	{
		palette_number = (pattern_info >> 9) & 0x30;
	
		if (pattern_info & 0x8000)
		{
			adj_value = VDP_PRIO_W + palette_number;
		}
		else
		{
			adj_value = filling_value + palette_number;
		}

		/* check for horizontal flipping */
		if (pattern_info & 0x0800)
		{
			revert_pattern();
		}

		process_pattern(dest, 3);
		process_pattern(dest, 2);
		process_pattern(dest, 1);
		process_pattern(dest, 0);
		process_pattern(dest, 7);
		process_pattern(dest, 6);
		process_pattern(dest, 5);
		process_pattern(dest, 4);
	}
	else
	{
		/* Poke the filling value */
		S32 i;

		for(i=0;i<8;i++)
			MD_Screen[dest + i] = filling_value;
	}
}

static void putline_scra_win(const U32 dest)
{
	palette_number = (pattern_info >> 9) & 0x30;

	/* check for horizontal flipping */
	if (pattern_info & 0x0800)
	{
		revert_pattern();
	}

	if (pattern_info & 0x8000)
	{
 		mask_scroll_b_p1(dest);

		adj_value = palette_number + VDP_PRIO_W;

		process_pattern_scra_win(dest, 3);
		process_pattern_scra_win(dest, 2);
		process_pattern_scra_win(dest, 1);
		process_pattern_scra_win(dest, 0);
		process_pattern_scra_win(dest, 7);
		process_pattern_scra_win(dest, 6);
		process_pattern_scra_win(dest, 5);
		process_pattern_scra_win(dest, 4);
	}
	else
	{
		adj_value = palette_number;

		process_pattern_scra_win_p0(dest, 3);
		process_pattern_scra_win_p0(dest, 2);
		process_pattern_scra_win_p0(dest, 1);
		process_pattern_scra_win_p0(dest, 0);
		process_pattern_scra_win_p0(dest, 7);
		process_pattern_scra_win_p0(dest, 6);
		process_pattern_scra_win_p0(dest, 5);
		process_pattern_scra_win_p0(dest, 4);
	}
}

INLINE process_pattern_spr(const U32 dest, const U32 pos, const U32 priority)
{
	if (pattern_data & 0xf)
		putpixel_spr(dest, pos, pattern_data & 0xf, priority);

	pattern_data >>= 4;
}

static void putline_sprite(const U32 dest, const S32 priority)
{
	const U32 dst = dest + vdp_cur_line_pixel;

	process_pattern_spr(dst,3,priority);
	process_pattern_spr(dst,2,priority);
	process_pattern_spr(dst,1,priority);
	process_pattern_spr(dst,0,priority);
	process_pattern_spr(dst,7,priority);
	process_pattern_spr(dst,6,priority);
	process_pattern_spr(dst,5,priority);
	process_pattern_spr(dst,4,priority);
}

static U32 get_vcell_offset(const int scroll)
{
	U32 tmp;

	if (scroll == SCROLL_B)
	{
#if defined(DREAMCAST) || defined(_PSP)
		U16 *vsram_ptr = (U16 *)(VSRam + 2);
		tmp = VDP_Current_Line + ((((U32)vsram_ptr[0])|(((U32)vsram_ptr[1])>>16)) >> interlaced);
#else
		U32 *vsram_ptr = (U32 *)(VSRam + 2);
		tmp = VDP_Current_Line + (vsram_ptr[0] >> interlaced);
#endif
	}
	else
	{
		U32 *vsram_ptr = (U32 *) VSRam;

		tmp = vsram_ptr[(Data_Misc.Cell & V_Scroll_MMask) >> 1];
		tmp >>= interlaced;
		tmp += VDP_Current_Line;
	}

	Data_Misc.Line_7 = tmp & 7;
	tmp = (tmp >> 3) & V_Scroll_CMask;

	return tmp;
}

INLINE void process_tile(void (*putline_func)(U32), U32 *dest, const U16 *ScrAddr, U32 *H_Cell, const U32 V_Offset)
{
	pattern_info = get_pattern_info(ScrAddr, *H_Cell, V_Offset);
	pattern_data = get_pattern_data();

	putline_func(*dest);

	*H_Cell = (*H_Cell + 1) & H_Scroll_CMask;
	(*dest) += 8;	
}


/*******************************************************/
/* Process line scroll B                               */
/*******************************************************/
static void process_line_scroll_b(void)
{
	S32 i;
	U32 dest;
	U32 H_Cell_offset, V_Cell_offset;
	U32 next_cell;
	U32 v_offset;

	dest = vdp_cur_line_pixel + ((xoffset >> 16) & 7);
	next_cell = ((xoffset >> 16) ^ 0x3FF) >> 3;
	H_Cell_offset = next_cell & H_Scroll_CMask;
	Data_Misc.Cell = (next_cell & 1) - 2;
	V_Cell_offset = get_vcell_offset(SCROLL_B);
	i = H_Cell;

	v_offset = V_Cell_offset << H_Scroll_CMul;
	if (scroll_mode)
	{
		do
		{
			process_tile(putline, &dest, (U16 *)ScrB_Addr, &H_Cell_offset, v_offset);
			i--;
			Data_Misc.Cell++;
			update_yoffset(&V_Cell_offset, SCROLL_B);
			v_offset = V_Cell_offset << H_Scroll_CMul;
		}
		while(i >= 0);
	}
	else
	{
		do
		{
			process_tile(putline, &dest, (U16 *)ScrB_Addr, &H_Cell_offset, v_offset);
			i--;
		}
		while(i >= 0);
	}
}


// Render the window
static void process_line_window(U32 start, U32 length)
{
    U32 dest;
    U32 tmp;
    U32 tmp_start;
    U16 *pattern_adr;

    tmp = (VDP_Current_Line >> 3) << H_Win_Mul;
    dest = vdp_cur_line_pixel + (start * 8) + 8;
    pattern_adr = (U16*)(Win_Addr + (tmp * 2));
    Data_Misc.Line_7 = VDP_Current_Line & 7;

    tmp_start = start;

    // window loop
    do
    {
        pattern_info = pattern_adr[tmp_start];
        pattern_data = get_pattern_data();
		putline_scra_win(dest);
        tmp_start++;      // next pattern
        dest += 8;
        length--;
    }
	while (length != 0);
}

/*******************************************************/
/* Process line scroll A window                        */
/*******************************************************/
static void process_line_scroll_a_win(void)
{
    U32 h_cell_tmp = H_Cell;
    U32 win_xpos_tmp = Win_X_Pos;
    U32 V_Cell_offset;
    U32 H_Cell_offset;
    S32 cur_line_greater_equal_posy_win;    // = 1 when current line >= pos Y window, otherwise 0
    U32 dest, next_cell;
    S32 tmp;
	U32 window_at_bottom;

    cur_line_greater_equal_posy_win = ((VDP_Current_Line >> 3) >= Win_Y_Pos)?1:0;
	window_at_bottom = VDP_Reg.Win_V_Pos >> 7;

    if (cur_line_greater_equal_posy_win ^ window_at_bottom)
    {
        // mixed mode: scroll A + window

		h_cell_tmp -= win_xpos_tmp;

        if (VDP_Reg.Win_H_Pos & 0x80)
        {
            // win right
            Data_Misc.Start_W = win_xpos_tmp;
            Data_Misc.Lenght_W = h_cell_tmp;
            Data_Misc.Start_A = 0;
            Data_Misc.Lenght_A = win_xpos_tmp;
        }
        else
        {
            // win left
            Data_Misc.Start_W = 0;
            Data_Misc.Lenght_W = win_xpos_tmp;
            Data_Misc.Start_A = win_xpos_tmp;
            Data_Misc.Lenght_A = h_cell_tmp;
        }

#ifdef SPRA_OPT
 Data_Misc.Lenght_A--;
#endif

        if (Data_Misc.Lenght_A >= 0)
        {
			U32 v_offset;

			// scroll A
			// continua al final al render del window si es necesario
            Data_Misc.Mask = xoffset & 7;
            next_cell = (xoffset ^ 0x3FF) >> 3;
            Data_Misc.Cell = Data_Misc.Start_A + (next_cell & 1) - 2;
			H_Cell_offset = (next_cell + Data_Misc.Start_A) & H_Scroll_CMask;
			dest = vdp_cur_line_pixel + Data_Misc.Mask + (Data_Misc.Start_A*8);

			V_Cell_offset = get_vcell_offset(SCROLL_A);

			v_offset = V_Cell_offset << H_Scroll_CMul;
			if (scroll_mode)
			{
				do
				{
					process_tile(putline_scra_win, &dest, (U16 *)ScrA_Addr, &H_Cell_offset, v_offset);
					Data_Misc.Lenght_A--;	
					Data_Misc.Cell++;
					update_yoffset(&V_Cell_offset, SCROLL_A);
					v_offset = V_Cell_offset << H_Scroll_CMul;
				}
				while (Data_Misc.Lenght_A >= 0);
			}
			else
			{
				do
				{
					process_tile(putline_scra_win, &dest, (U16 *)ScrA_Addr, &H_Cell_offset, v_offset);
					Data_Misc.Lenght_A--;
				}
				while (Data_Misc.Lenght_A >= 0);
			}

#ifdef SPRA_OPT
			pattern_info = get_pattern_info((U16 *)ScrA_Addr, H_Cell_offset,V_Cell_offset);
			pattern_data = get_pattern_data();

			// horizontal flipping?
			if (pattern_info & 0x0800)
			{
				pattern_data &= Mask_F[Data_Misc.Mask];
			}
			else
			{
				pattern_data &= Mask_N[Data_Misc.Mask];
			}

			putline_scra_win(dest);
#endif

            // Check whether or not we have to continue with window rendering
            if ((Data_Misc.Lenght_W & 0xFF) == 0) return;
        }
         process_line_window(Data_Misc.Start_W,Data_Misc.Lenght_W);
    }
    else
    {
        // full window
         process_line_window(0,H_Cell);
    }
}

/****************************************/
/* Process line of sprite layer         */
/****************************************/
static void process_line_spr(void)
{
	U32 *vram_ptr = (U32 *) VRam;
	S32 i;
	U32 pattern_line;
	S32 ysize, yoffset;		// EBX = ysize
	S32 next_cell;
	S32 spr_idx;			 // EDI
	U32 cell_info;
	S32 num_pattern;		// EDX -> luego palnum
	S32 dest;
	S32 tmp;
	U32 first_pat;


	// update the sprite masking
	update_spr_mask(Sprite_Over & 1);

	// Sprite loop
	for(i=0;i<Data_Misc.Borne;i++)
	{
		// First loop
		spr_idx = Sprite_Visible[i + 0];			// Sprite index to sprite struct
		cell_info = Sprite_Struct[spr_idx + 6];		// Cell information

// print_cell_info(cell_info);

		tmp = Sprite_Struct[spr_idx + 1];		// Line - Y Pos (Y Offset)
		yoffset = VDP_Current_Line - tmp;

		first_pat = cell_info & 0x7FF;			// number of the first sprite pattern
		Data_Misc.Palette = (cell_info >> 9) & 0x30;	// Palette number * 16
		num_pattern = yoffset & 0xF8;			// pattern number * 8
		ysize = Sprite_Struct[spr_idx + 3];		// Y size
		pattern_line = yoffset & 7;			// Current pattern line
	
		ysize <<= (3 + interlaced);
		num_pattern <<= interlaced;
		first_pat <<= (3 + interlaced);

// print_first_pat(first_pat);

		if (cell_info & 0x1000)		// v-flip?
		{
//print_tag(2);
			// Vertical flip
			ysize -= num_pattern;
			pattern_line ^= 7;
			first_pat += ysize;
	
			ysize += (num_pattern + (1 << (3 + interlaced)));
		}
		else
		{
//print_tag(3);
			// No vertical flip
			first_pat += num_pattern;
			ysize += (1 << (3 + interlaced));
		}

		first_pat += (pattern_line << interlaced);
	
		// Suite
		Data_Misc.Next_Cell = ysize;
		palette_number = Data_Misc.Palette;	// Palette number * 64
	
		if (cell_info & 0x0800)		// Test horizontal flipping
		{
			// Horizontal flip
			dest = Sprite_Struct[spr_idx + 4] - 7;
			Data_Spr.H_Min = Sprite_Struct[spr_idx + 0];

			if (Data_Spr.H_Min < -7)
				Data_Spr.H_Min = -7;

			while (dest >= H_Pix)
			{
				first_pat += Data_Misc.Next_Cell;
				dest -= 8;
			}

			do
			{
				pattern_data = vram_ptr[first_pat];
				if (pattern_data)
				{
					revert_pattern();
					putline_sprite(dest, (cell_info & 0x8000) >> 15);
				}
				first_pat += Data_Misc.Next_Cell;
				dest -= 8;
			}
			while (dest >= Data_Spr.H_Min);
		}
		else
		{
			// No horizontal flip
			dest = Sprite_Struct[spr_idx + 0];

			// borne max = sprite max
			Data_Spr.H_Max = Sprite_Struct[spr_idx + 4];

			// borne max = sprite max / clip screen
			if (Data_Spr.H_Max > H_Pix)
			{
				Data_Spr.H_Max = H_Pix;
			}

			while (dest < -7)
			{
				dest += 8;
				first_pat += Data_Misc.Next_Cell;
			}

			do
			{
				pattern_data = vram_ptr[first_pat];
				if (pattern_data)
					putline_sprite(dest, (cell_info & 0x8000) >> 15);
				first_pat += Data_Misc.Next_Cell;
				dest += 8;
			}
			while (dest < Data_Spr.H_Max);
		}
	}
}

/****************************************/
/* Process line                         */
/****************************************/
static void process_line(void)
{
	interlaced = (VDP_Reg.Set4 & 0x4) >> 2;		/* bit 2 = interlaced */
	hishadow = VDP_Reg.Set4 & 0x8;				/* bit 3 = highlight/shadow */

	/* scroll_mode: 0 = full scroll; 1 = vertical scroll   */
	scroll_mode = VDP_Reg.Set3 & 4;

	xoffset = get_xoffset();

	/* Setting up background filling colour */
	if (hishadow)
	{
		filling_value = VDP_SHADOW_W;
	}
	else
	{
		filling_value = 0;
	}

 	process_line_scroll_b();
	process_line_scroll_a_win();
  	process_line_spr();
}


/*****************************************************************************/
/* Render API functions                                                      */
/*****************************************************************************/

/* Main Genesis/Sega CD render function */
void Render_Line(void)
{
	return;
	U32 i, offset;

    /* This is the offset of the first pixel in the current line */
	vdp_cur_line_pixel = (VDP_Current_Line << 8) + (VDP_Current_Line << 6) + 
                        (VDP_Current_Line << 4); /* VDP_Current_Line * 336 */

	/* Test to see if VDP is enabled */
	if (VDP_Reg.Set2 & 0x40)
	{
		/* VDP enabled */
		switch( VRam_Flag + (VDP_Reg.Set4 & 4) )
		{
			case 0:
			case 4:
				/* Sprite struct does not need to be built */
				break;

			case 1:
			case 3:
				/* Sprite struct needs to be built (no interlaced) */
				build_spr_struct(0);
				break;

			case 5:
			case 7:

				/* Sprite struct needs to be built (interlaced) */
				build_spr_struct(1);
				break;

			case 2:
			case 6:
				/* Sprite struct needs to be built partially (no interlaced) */
				build_spr_struct_partial();
				break;
		}

		/* Clear VRam flag */
		VRam_Flag = 0;

		process_line();
	}
	else
	{
		/* VDP disabled */
		const U32 pixdata = (VDP_Reg.Set4 & 0x8)?VDP_SHADOW_D:0;
		S32 * scr_ptr = (S32 *)MD_Screen;

		/* Dump the line data */
		for(i=0;i<320/2;i++)
			scr_ptr[((vdp_cur_line_pixel + 8)/2) + i] = pixdata;
	}

	if (CRam_Flag)
	{
		/* Palette modified */
		update_palette( VDP_Reg.Set4 & 8 );
	}


	offset = vdp_cur_line_pixel + 8;

	/* Start the render itself */
	for(i=0;i<(320-H_Pix_Begin*2);i++)
	{
		U8 tmp;
		U8 *scr_ptr = (U8 *)MD_Screen;

		tmp = scr_ptr[(offset + i)*2];
		MD_Screen[offset + i] = MD_Palette[tmp];
	}
}

void Render_Line_32X()
{
}
