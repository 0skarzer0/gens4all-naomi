
#include "gens4all.h"
#include "vdp_rend.h"
#include "blit.h"

#ifndef U32
	#define U32 unsigned int
#endif

#ifndef DREAMCAST
void Blit_X1(unsigned char *Dest, const int pitch, const int x, const int y, const int offset)
{
	int i;
	register U32 *s = (U32 *) MD_Screen + 4;
	register U32 *d = (U32 *)Dest;
	const U32 nx = (x >> 4);
	const U32 soff = (offset/4);
	const U32 doff = (pitch - x*2)/4;

	for (i = 0; i < y; i++)
	{
		register int n=nx;
		while(n--)
		{
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
		}
		s += soff;
		d += doff;
	}
}

void Blit_Scale2x (unsigned char *Dest, int pitch, int x, int y, int offset) 
{
	int i, j;
	int B, D, E, F, H;
	int E0, E1, E2, E3;
  
    // A B C
    // D E F
    // G H I
  
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;

	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	SrcOffs = 8;
	DstOffs = 0;
  
	for (i = 0; i < y; i++)
    {
		E = MD_Screen[SrcOffs - 1];
		F = MD_Screen[SrcOffs];
		DstOffs = i * pitch * 2;
		
		for (j = 0; j < x; j++)
		{
			D = E;
			E = F;
			B = MD_Screen[SrcOffs - ScrWdth];
			H = MD_Screen[SrcOffs + ScrWdth];
			F = MD_Screen[++SrcOffs];
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;
	  
			// E0
			Dest[DstOffs] = E0;
			Dest[DstOffs + 1] = E0 >> 8;
	  
			// E1
			Dest[DstOffs + 2] = E1;
			Dest[DstOffs + 3] = E1 >> 8;
	  
			// E2
			Dest[DstOffs + pitch] = E2;
			Dest[DstOffs + pitch + 1] = E2 >> 8;
	  
			// E3
			Dest[DstOffs + pitch + 2] = E3;
			Dest[DstOffs + pitch + 3] = E3 >> 8;

			DstOffs += 4;
		}     

		SrcOffs += ScrAdd;
	}
}


#else

#if 1
#include "../../../libsh4/sh4.h"
#include "../../../libsh4/sq.h"
//#include "../pvrvdp/pvr_cxt.h"
#include "../pvrvdp/npvr_vertex.h"
#include "../pvrvdp/pvr_rend.h"



void PrintTimer() {
	const int sectowait = 10;
	static int tick = sectowait*60;
	static int sec = 0;
	if (--tick == 0) {
		tick = sectowait*60;
		sec += sectowait;
		printf("%i sec. ", sec);
		fflush(stdout);
	}
}

#include "vdp_io.h"

PVRVRAMData __attribute__ ((aligned(32))) VdpTiles;

void DrawMenuBlend() {
	
}
#ifdef SKIPLOADER
int Show_Timing = 1;
int Show_Rasters = 1;
#else
int Show_Timing = 0;
int Show_Rasters = 0;
#endif

void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset)
{
	//PrintTimer();
//	return;
	int i;
	extern long long last_frame_duration_us;
	
	static long long f2f_last_frame_start_us = 0;
	static long long f2f_last_frame_p2p_start_us = 0;
	static long long last_reg_time_us = 0;
	
	static int fskipped = 0;
	
	long long f2f_last_frame_duration_us = timer_us_gettime64() - f2f_last_frame_start_us;
	//~ printf("%.2f ms\n", f2f_last_frame_duration_us / 1000.f);
	//~ f2f_last_frame_start_us = timer_us_gettime64();
	
//	extern int frameskipping;
//	if (frameskipping) {
//		return;
//	}
	
	//*
	#define frameskiptime (17.0*1000)
	if (0 && !fskipped && ((last_frame_duration_us > frameskiptime) || (f2f_last_frame_duration_us > frameskiptime))) {
		//printf("S"); fflush(stdout);
		fskipped = 1;
		return;
	}
	fskipped = 0;
	
	/*
	if (!fskipped && (vbls > 1)) {
		//~ printf("S"); fflush(stdout);
		fskipped = 1;
		return;
	}
	fskipped = 0;
	//*/
	
//	vid_waitvbl();
//	return;
	
	short *cram = (short *)Raster0_CRam;
	int vdpbgcolor = cram[VDP_Reg.BG_Color];
	int red = vdpbgcolor & 0xe;
	int gre = (vdpbgcolor>>4) & 0xe;
	int blu = (vdpbgcolor>>8) & 0xe;
	red <<= 4;
	gre <<= 4;
	blu <<= 4;
	vdpbgcolor = 0xff000000 | (red<<16) | (gre<<8) | blu;
//	red |= red >> 3;
//	gre |= gre >> 3;
//	blu |= blu >> 3;
//	red |= red >> 3;
//	gre |= gre >> 3;
//	blu |= blu >> 3;
//	vid_border_color(red, gre, blu);
	
	pvr_wait_ready();
	long long reg_time_us = timer_us_gettime64();
	pvr_stats_t stats;
	pvr_get_stats(&stats);
	
	
	static int lastvbl = 0;
	int vbls = stats.vbl_count - lastvbl;
	lastvbl = stats.vbl_count;
	
	long long f2f_last_frame_duration_p2p_us = timer_us_gettime64() - f2f_last_frame_p2p_start_us;
	f2f_last_frame_p2p_start_us = timer_us_gettime64();
	f2f_last_frame_start_us = timer_us_gettime64();
	
prof_start(PROF_VRENDER);
	//Load VRAM texture
	if (0) {
		CurPVRBuff ^= 1;
		CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
		CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
		VRAMToTexture(VRam, PVRTilesTexture[CurPVRBuff]);
		//~ pvr_txr_load_dma(&VdpTiles, PVRTilesTexture[CurPVRBuff], sizeof(VdpTiles), 1, 0, 0);
	}
	if (1) {
		long long texgenstart = timer_us_gettime64();
		
		long long cbtime = 0;
		long long vramtime = 0;
		long long flushtime = 0;
		long long dmatime = 0;
		if (0) {
			long long cbstart = timer_us_gettime64();
			PalToCodebook2(cram,    (void*)VdpTiles.CB[0].VDPPalette[0]);
			PalToCodebook2(cram+16, (void*)VdpTiles.CB[0].VDPPalette[1]);
			PalToCodebook2(cram+32, (void*)VdpTiles.CB[0].VDPPalette[2]);
			PalToCodebook2(cram+48, (void*)VdpTiles.CB[0].VDPPalette[3]);
			cbtime = timer_us_gettime64() - cbstart;
			//~ PalToCodebook(cram+16, (void*)VdpTiles.PaletteCodebook+2048*1);
			//~ PalToCodebook(cram+32, (void*)VdpTiles.PaletteCodebook+2048*2);
			//~ PalToCodebook(cram+48, (void*)VdpTiles.PaletteCodebook+2048*3);
			cbstart = timer_us_gettime64();
			//~ VRAM8x8ToTexture2(VRam, OCI_BANK1(VdpTiles.VRAM));
			VRAM8x8ToTexture2(VRam, sqPrepare(VdpTiles.VRAM));
			//~ VRAMToTexture(VRam, VdpTiles.VRAM);
			vramtime = timer_us_gettime64() - cbstart;
			long long flushstart = timer_us_gettime64();
			//~ purge_cache_oci();
			flushtime = timer_us_gettime64() - flushstart;
			
			CurPVRBuff ^= 1;
			CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
			CurPVRTilesCxtAlt = &PVRTilesCxtBnkAlt[CurPVRBuff][0];
			CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
			CurPVRTilesSprCxtAlt = &PVRTilesSprCxtBnkAlt[CurPVRBuff][0];
			long long dmastart = timer_us_gettime64();
			pvr_txr_load_dma(&VdpTiles, PVRTilesTexture[CurPVRBuff], sizeof(VdpTiles), 1, 0, 0);
			dmatime = timer_us_gettime64() - dmastart;
			
			
		} else {
			CurPVRBuff ^= 1;
			CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
			CurPVRTilesCxtAlt = &PVRTilesCxtBnkAlt[CurPVRBuff][0];
			CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
			CurPVRTilesSprCxtAlt = &PVRTilesSprCxtBnkAlt[CurPVRBuff][0];
			
			//~ PalToCodebook2(cram,    sqPrepare((void*)VdpTiles.PaletteCodebook+2048*0));
			//~ PalToCodebook2(cram+16, sqPrepare((void*)VdpTiles.PaletteCodebook+2048*1));
			//~ PalToCodebook2(cram+32, sqPrepare((void*)VdpTiles.PaletteCodebook+2048*2));
			//~ PalToCodebook2(cram+48, sqPrepare((void*)VdpTiles.PaletteCodebook+2048*3));
			//~ VRAM8x8ToTexture2(VRam, sqPrepare(VdpTiles.VRAM));
			uintptr_t texstartint = (uintptr_t)PVRTilesTexture[CurPVRBuff];
			texstartint = 0x11000000 | (texstartint & 0xffffff);
			PVRVRAMData *texstart = (PVRVRAMData*)texstartint;
			
			struct pvr_dma_mode_reg_layout {
				int mode0, mode1;
			};
			#define PVR_DMA_FAST_MODE (*(volatile struct pvr_dma_mode_reg_layout*)0xA05F6884)
			PVR_DMA_FAST_MODE.mode0 = PVR_DMA_FAST_MODE.mode1 = 0;
			
			long long cbstart = timer_us_gettime64();
			if (1 && Raster_ColorSplit) {
				short *cram2 = (short *)Raster1_CRam;
				PalToCodebook4(cram2,    sqPrepare(texstart->CB[1].VDPPalette[0]));
				PalToCodebook4(cram2+16, sqPrepare(texstart->CB[1].VDPPalette[1]));
				PalToCodebook4(cram2+32, sqPrepare(texstart->CB[1].VDPPalette[2]));
				PalToCodebook4(cram2+48, sqPrepare(texstart->CB[1].VDPPalette[3]));
			}
			PalToCodebook4(cram,    sqPrepare(texstart->CB[0].VDPPalette[0]));
			PalToCodebook4(cram+16, sqPrepare(texstart->CB[0].VDPPalette[1]));
			PalToCodebook4(cram+32, sqPrepare(texstart->CB[0].VDPPalette[2]));
			PalToCodebook4(cram+48, sqPrepare(texstart->CB[0].VDPPalette[3]));
			cbtime = timer_us_gettime64() - cbstart;
			cbstart = timer_us_gettime64();
			VRAM8x8ToTexture2(VRam, sqPrepare(texstart->VRAM));
			//~ VRAM8x8ToTexture2(VRam, OCI_BANK1(texstart->VRAM));
			vramtime = timer_us_gettime64() - cbstart;
			//~ printf("%08x (%i)\n", r, r - ((uintptr_t)vdst)); fflush(stdout);
			//~ VRAMToTexture(VRam, VdpTiles.VRAM);
			//~ purge_cache();
		}
		if (0) {
			long long texgentime = timer_us_gettime64() - texgenstart;
			float cb_ms = (int)cbtime / 1000.0f;
			float vram_ms = (int)vramtime / 1000.0f;
			float dma_ms = (int)dmatime / 1000.0f;
			float flush_ms = (int)flushtime / 1000.0f;
			float texgen_ms = (int)texgentime / 1000.0f;
			printf("CB %f ms\n", cb_ms);
			printf("VRAM %f ms\n", vram_ms);
			printf("Flush %f ms\n", flush_ms);
			printf("DMA %f ms\n", dma_ms);
			printf("Texgen %f ms\n", texgen_ms);
		}
	}
	
	extern struct Reg_VDP_Type VDP_Reg;
	extern int H_Pix;
	
	//Render screen
	pvr_scene_begin();
		void *pc = sqPrepare((void*)PVR_TA_INPUT);
		
		//Cover up any VDP rendering with the bg color if the screen is off
		pvr_list_begin(PVR_LIST_OP_POLY);
			{ //Force opaque list for normal KOS PVR driver
			pvr_context spr;
			pc_set_default_sprite(&spr);
			pc_set_list(&spr, PC_OPAQUE_POLY);
			pc_set_modified(&spr, 1);
			pc_set_modifier_full(&spr, 0);
			sqCopy32(pc, &spr);
			}
			//TODO Get colors from Raster_BgColor
			for(i = 0; i < 224; i++) {	//TODO PAL 240 line mode
				draw_rect_wh(screen_start_x, SCREEN_START_Y + i*TSH, H_Pix*TSW, TSH,
					(Raster_Lines[i] & RASTER_LINE_SCREEN_OFF) ? DEPTH_LAYER_S_HI + 0.5 : DEPTH_LAYER_BG,
					//~ DEPTH_LAYER_BG,
					vdpbgcolor, pc);
			}
		pvr_list_finish();
		
		pvr_list_begin(PVR_LIST_OP_MOD);
			extern void DrawMapSH();
			DrawMapSH();
		pvr_list_finish();
		
		pvr_list_begin(PVRVDP_TARGET_LIST == PC_PUNCHTHROUGH ? PVR_LIST_PT_POLY : PVR_LIST_TR_POLY);
		
		short *sprlist = (short*)(((VDP_Reg.Spr_Att_Adr & 0x7F) << 9) + VRam);
		
		//*
#ifdef VDP_HIRES
		screen_start_x = H_Pix == 320 ? 0 : (640-512)/2;
		//~ draw_rect_wh(screen_start_x, SCREEN_START_Y, H_Pix*2, 224*2, DEPTH_LAYER_BG, 0xff000000 | (red<<16) | (gre<<8) | blu, pc);
#else
		screen_start_x = H_Pix == 320 ? 0 : (320-256)/2;
		//~ draw_rect_wh(screen_start_x, SCREEN_START_Y, H_Pix, 224, DEPTH_LAYER_BG, 0xff000000 | (red<<16) | (gre<<8) | blu, pc);
#endif
		//*/
		//Is VDP enabled?
		extern int Force_Screen_On_Hack;
		if (1 || (Force_Screen_On_Hack || (VDP_Reg.Set2 & 0x40))) {
			//Draw screen
			DrawMapWindow(DEPTH_LAYER_A_LO+0.1, DEPTH_LAYER_A_HI+0.1);
			DrawMapPVR(0, DEPTH_LAYER_A_LO, DEPTH_LAYER_A_HI);
			DrawMapPVR(1, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI);
			DrawSprites();
		}
		
		//Screen border
		//#define SCREEN_BORDER_COLOR	0xc0300000
		#define SCREEN_BORDER_COLOR	0xff000000
		#define SCREEN_BORDER_WIDTH	64
#ifndef VDP_HIRES
		draw_rect_wh(screen_start_x - SCREEN_BORDER_WIDTH, SCREEN_START_Y-SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 224+SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Left edge
		draw_rect_wh(H_Pix + screen_start_x, SCREEN_START_Y, SCREEN_BORDER_WIDTH, 224, 15, SCREEN_BORDER_COLOR, pc);	//Right edge
		draw_rect_wh(screen_start_x, SCREEN_START_Y-SCREEN_BORDER_WIDTH, H_Pix+SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Top edge
		draw_rect_wh(screen_start_x - SCREEN_BORDER_WIDTH, SCREEN_START_Y+224, H_Pix+SCREEN_BORDER_WIDTH*2, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Bottom edge
#endif
		//~ int i;

	if (Show_Rasters) {
		//Show raster lines
		for(i = 0; i < 224; i++) {
			if (Raster_Lines[i]) {
				int color = 0xff202020;
				color |= (Raster_Lines[i] & RASTER_LINE_COLOR_CHANGE) ? 0xff0000 : 0;
				color |= (Raster_Lines[i] & RASTER_LINE_SCROLL_CHANGE) ? 0x00ff00 : 0;
				color |= (Raster_Lines[i] & RASTER_LINE_SPRLIST_CHANGE) ? 0x0000ff : 0;
				draw_rect_wh(screen_start_x, SCREEN_START_Y+i, 8, 1, 15, color, pc);
			}
		}
	}
		//if (Raster_ColorSplit)
		//	draw_rect_wh(screen_start_x, SCREEN_START_Y + Raster_ColorSplitLine-1, 3, 3, 20, 0xff00ffff, pc);
prof_end(PROF_VRENDER);
		
		//Profile gauge
	#if 0
		#define PROF_TICK_VPOS	(230.0f)
		#define PROF_TICK_PIXELS	(150.0f)
	#else
	#if 0
		#define PROF_TICK_VPOS	(400.0f)
		#define PROF_TICK_PIXELS	(200.0f)
	#else
		#define PROF_TICK_VPOS	(225.0f)
		#define PROF_TICK_PIXELS	(100.0f)
	#endif
	#endif
	
	#define WHITE 0xffffffff
	#define GREEN 0xff00ff00
	#define DARK_GREEN 0xff008000
	#define RED 0xffa00000

	if (Show_Timing) {
		//float frametime_ms = last_frame_duration_us / 1000.0f;
		#define frame_length	(16.666f)
		#define us2ms(a) ((a) / 1000.0f)
		#define bar(time) ((time) / frame_length * PROF_TICK_PIXELS)
		#define bar_us(time) bar(us2ms(time))
		draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(f2f_last_frame_duration_us), 8, 19.5, GREEN, pc);	//f2f time
		//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar_us(f2f_last_frame_duration_p2p_us), 8, 19.5, 0xff008000, pc);	//f2f time
		//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(stats.frame_last_time), 8, 19.5, 0xff008000, pc);	//f2f time
		draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(vbls * frame_length), 8, 19.5, 0xff008000, pc);	//f2f time
		draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar((float)stats.rnd_last_time), 8, 19.7, RED, pc);	//render
		draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(prof_last_frame_time[PROF_M68K]), 3, 19.7, 0xffff0000, pc);	//68k time
		draw_rect_wh(0,PROF_TICK_VPOS+5, bar_us(prof_last_frame_time[PROF_YMH]), 3, 19.7, 0xff4040ff, pc);	//fm synth
		draw_rect_wh(0,PROF_TICK_VPOS+8, bar_us(last_reg_time_us), 3, 19.8, 0xffa040a0, pc);	//register
		//~ draw_rect_wh(0,PROF_TICK_VPOS+8, bar_us(prof_last_frame_time[PROF_VRENDER]), 3, 19.7, 0xffa040a0, pc);	//render
		draw_rect_wh(0,PROF_TICK_VPOS+11, bar_us(prof_last_frame_time[PROF_Z80]), 3, 19.7, 0xffb0b0b0, pc);	//z80
		
		for(i = 0; i < 17; i++) {
			float h = (i % 5 != 0) ? 4 : 8;
			draw_rect_wh(i * (PROF_TICK_PIXELS/frame_length),PROF_TICK_VPOS, 1, h, 20, WHITE, pc);	//millisec
		}
		draw_rect_wh(PROF_TICK_PIXELS,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//1 frame
		draw_rect_wh(PROF_TICK_PIXELS*2,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//2 frame
		draw_rect_wh(PROF_TICK_PIXELS*3,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//3 frame
	}
	if (PVRVDP_TARGET_LIST == PC_PUNCHTHROUGH) {
		pvr_list_finish();
		pvr_list_begin(PVR_LIST_TR_POLY);
	}
		if (Paused) {
			void *pc = sqPrepare((void*)PVR_TA_INPUT);
			
			{ //Force transparent list for normal KOS PVR driver
			pvr_context spr;
			pc_set_default_sprite(&spr);
			pc_set_list(&spr, PC_BLEND_POLY);
			sqCopy32(pc, &spr);
			}
			
			//Process and draw in-game pause menu
			extern void RunGameMenu();
			RunGameMenu();
		}
		
		pvr_list_finish();
	pvr_scene_finish();
	
	reg_time_us = timer_us_gettime64() - reg_time_us;
	last_reg_time_us = reg_time_us;
}

#else


void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset)
{
	int i;

	register U32 *s = (U32 *) MD_Screen + 4;
	register U32 *d = (U32 *)(void *) (0xe0000000 | (((unsigned long)Dest) & 0x03ffffe0));
	const U32 nx = (x >> 4);
	const U32 soff = (offset/4);
	const U32 doff = (640 - x*2)/4;

	QACR0 = ((((unsigned int)Dest)>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)Dest)>>26)<<2)&0x1c;

	for(i=0;i<y;i++)
	{
		register int n=nx;
		while(n--)
		{
			asm("pref @%0" : : "r" (s + 8));
			d[0] = *(s++);
			d[1] = *(s++);
			d[2] = *(s++);
			d[3] = *(s++);
			d[4] = *(s++);
			d[5] = *(s++);
			d[6] = *(s++);
			d[7] = *(s++);
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		s+=soff;
		d+=doff;
	}
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}
#endif

#endif
