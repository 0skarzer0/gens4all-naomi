/*************************************/
/* Module to manage genesis stuff    */
/*************************************/

/*************************************/
/*              GENESIS              */
/*************************************/

#include "gens4all.h"
#include "misc.h"
#include "util/ggenie.h"

extern Uint32 Last_Time;
extern Uint32 New_Time;
extern Uint32 Used_Time;


/* Loads Genesis BIOS */
void Init_Genesis_Bios (void)
{
	FILE *fp;

	if ((fp = fopen (Genesis_Bios, "rb")))
	{
		fread (&Genesis_Rom[0], 1, 2 * 1024, fp);
		Byte_Swap (&Genesis_Rom[0], 2 * 1024);
		fclose (fp);
	}
	else
		memset (Genesis_Rom, 0, 2 * 1024);

	Rom_Size = 2 * 1024;
	memcpy (Rom_Data, Genesis_Rom, 2 * 1024);
	Game_Mode = 0;
	CPU_Mode = 0;
	VDP_Num_Vis_Lines = 224;
	M68K_Reset (0);
	Z80_Reset ();
	Reset_VDP ();
	CPL_Z80 = Round_Double ((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
	CPL_M68K = Round_Double ((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
	VDP_Num_Lines = 262;
	VDP_Status &= 0xFFFE;
	YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
}

/* Initialise Genesis system */
int Init_Genesis (struct Rom *MD_Rom)
{
	char Str_Err[256];
	char *rom_name;

	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;

#ifdef KAILLERA_SUPPORT
	if (!Kaillera_Client_Running)
#endif
	{
		if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A')
		&& (MD_Rom->Ram_Infos[10] & 0x40))
		{
			SRAM_Start = MD_Rom->Ram_Start_Adress & 0x0F80000;	// multiple de 0x080000
			SRAM_End = MD_Rom->Ram_End_Adress;
		}
		else
		{
			SRAM_Start = 0x200000;
			SRAM_End = 0x200000 + (64 * 1024) - 1;
		}

		if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
			SRAM_End = SRAM_Start + (64 * 1024) - 1;

		if (Rom_Size <= (2 * 1024 * 1024))
		{
			SRAM_ON = 1;
			SRAM_Write = 1;
		}

		SRAM_Start &= 0xFFFFFFFE;
		SRAM_End |= 0x00000001;

		if ((SRAM_End - SRAM_Start) <= 2)
		{
			SRAM_Custom = 1;
		}
		else
		{
			SRAM_Custom = 0;
		}
		
		Load_SRAM ();
	}

  switch (Country)
    {
    default:
    case -1:
      Detect_Country_Genesis ();
      break;

    case 0:
      Game_Mode = 0;
      CPU_Mode = 0;

      break;

    case 1:
      Game_Mode = 1;
      CPU_Mode = 0;
      break;

    case 2:
      Game_Mode = 1;
      CPU_Mode = 1;
      break;

    case 3:
      Game_Mode = 0;
      CPU_Mode = 1;
      break;
    }

	/* Buscar nombre de ROM valido */
	rom_name = (MD_Rom->Rom_Name_W[0] != '\0') ? MD_Rom->Rom_Name_W : MD_Rom->Rom_Name;

	//sprintf (Str_Err, "Gens :: %s [%s]", rom_name,MD_Rom->Console_Name);
	//SetWindowText (Str_Err);

	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)

	Byte_Swap (Rom_Data, Rom_Size);
	
	M68K_Reset (0);
	Z80_Reset ();
	Reset_VDP ();
	
	if (strstr(MD_Rom->Version,"T-5740"))
	{
		Init_PS_mapper();
	}

	if (CPU_Mode)
    {
		CPL_Z80 = Round_Double ((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double ((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);

#ifdef SEGA_32X
		CPL_MSH2 = Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) SSH2_Speed) / 100.0);
#endif
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		
		YM2612_Init (CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init (CLOCK_PAL / 15, Sound_Rate);
    }
	else
    {
		CPL_Z80 = Round_Double ((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double ((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		
#ifdef SEGA_32X
		CPL_MSH2 = Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) SSH2_Speed) / 100.0);
#endif

		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;

		YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
	}

	if (Auto_Fix_CS)
	{
		Fix_Checksum ();
	}

	dbgf("Init_Genesis: Sound_Enable is %i\n", Sound_Enable);
	
	if (Sound_Enable)
	{
		dbg("Initing_Sound");
		End_Sound ();
		
		if (!Init_Sound ()) 
		{
			Sound_Enable = 0;
			dbg("Could not init sound, disabling it");
		} 
		else
		{
			Play_Sound ();
		}
	}


	Load_Patch_File ();
	
	Last_Time = GetTickCount ();
	New_Time = 0;
	Used_Time = 0;
	
	extern int Do_Genesis_Frame_No_Raster(void);
	Update_Frame = Do_Genesis_Frame;
	Update_Frame_Fast = Do_Genesis_Frame_No_VDP;
	
	return 1;
}

/* Reset Genesis system */
void Reset_Genesis(void)
{
	Controller_1_COM = Controller_2_COM = 0;
	Paused = 0;
	
	if (Rom_Size <= (2 * 1024 * 1024))
	{
		SRAM_ON = 1;
		SRAM_Write = 1;
	}
	else
	{
		SRAM_ON = 0;
		SRAM_Write = 0;
	}

	M68K_Reset (0);

	Z80_Reset ();
	Reset_VDP ();
	YM2612_Reset ();

	
	if (CPU_Mode)
		VDP_Status |= 1;
	else
		VDP_Status &= ~1;
	
	if (Auto_Fix_CS)
		Fix_Checksum ();
}

int frameskipping = 0;
extern int inkey;
int Do_Genesis_Frame_No_Raster(void)
{
	frameskipping = 0;
	prof_inc_frame_count();
	int *buf[2];
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
		VDP_Num_Vis_Lines = 240;
	else
		VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
//  main68k_tripOdometer ();
	M68K_ResetCyclesCounter();

	Z80_Clear_Clock();

	Patch_Codes ();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H

	memcpy(Raster0_CRam, CRam, sizeof(Raster0_CRam));
	memset(Raster_Lines, 0, sizeof(Raster_Lines));
	memcpy(Raster_Sprites[0], (void*)Spr_Addr, sizeof(Raster_Sprites[0]));
	Raster_ColorSplit = 0;
	Raster_ColorSplitLine = 1<<17;
	Raster_SpriteSplitLine = 224;

	for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
			 VDP_Current_Line++)
		{
			//~ CRam_Line_Dirty = 0;
			//~ VSRam_Line_Dirty = 0;
			//~ Raster_Lines[VDP_Current_Line] = 0;
			
			buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
			buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
			YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
			YM_Len += Sound_Extrapol[VDP_Current_Line][1];
			PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

			Fix_Controllers ();
			Cycles_M68K += CPL_M68K;
			Cycles_Z80 += CPL_Z80;
			if (DMAT_Lenght)
//				main68k_addCycles (Update_DMA ());
				M68K_AddCycles(Update_DMA ());

			VDP_Status |= 0x0004;	// HBlank = 1
//      main68k_exec (Cycles_M68K - 404);
			M68K_Exec(Cycles_M68K - 404);

			VDP_Status &= 0xFFFB;	// HBlank = 0

			if (--HInt_Counter < 0)
			{
				HInt_Counter = VDP_Reg.H_Int;
				VDP_Int |= 0x4;
				Update_IRQ_Line ();
			}

			

//      prof_start(PROF_VRENDER);
//      Render_Line ();
//      prof_end(PROF_VRENDER);
	
			M68K_Exec(Cycles_M68K);
			Z80_Process(Cycles_Z80);
		//*	
			
			
			{
				Raster_BgColor[VDP_Current_Line] = ((short*)CRam)[VDP_Reg.BG_Color];
				
				if (Raster_Lines[VDP_Current_Line] & RASTER_LINE_SPRLIST_CHANGE)
					Raster_SpriteSplitLine = VDP_Current_Line;
			
				if ((Raster_Lines[VDP_Current_Line] & RASTER_LINE_COLOR_CHANGE) && !Raster_ColorSplit) {
					Raster_ColorSplit = 1;
					Raster_ColorSplitLine = VDP_Current_Line;
					//~ memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
				}
				
				if (!(VDP_Reg.Set2 & 0x40)) {	//Is screen off?
					Raster_Lines[VDP_Current_Line] |= RASTER_LINE_SCREEN_OFF;
				}// else
				//~ if (VDP_Reg.Set3 & 0x4) {
					//~ //Read from center of screen when vert cell scroll is on
					//~ Raster_Data[VDP_Current_Line].vscroll_a = ((short*)&VSRam)[20];
					//~ Raster_Data[VDP_Current_Line].vscroll_b = ((short*)&VSRam)[21];
				//~ } else {
					extern u32 V_Scroll_CMask;
					//~ int vmask = (V_Scroll_CMask << 3) | 0x7;
					int vmask = 0x3ff;
					Raster_Data[VDP_Current_Line].vscroll_a = ((short*)&VSRam)[0]&vmask;
					Raster_Data[VDP_Current_Line].vscroll_b = ((short*)&VSRam)[1]&vmask;
					
				//~ }
				extern int H_Scroll_Mask;
				extern unsigned int H_Scroll_Addr;
				short *hscrolltable = (short*)H_Scroll_Addr;
				
				Raster_Data[VDP_Current_Line].hscroll_a = hscrolltable[(VDP_Current_Line&H_Scroll_Mask)*2];
				Raster_Data[VDP_Current_Line].hscroll_b = hscrolltable[(VDP_Current_Line&H_Scroll_Mask)*2+1];
			}
			//*/
			
		}
	
	if (Raster_SpriteSplitLine < 224)
		memcpy(Raster_Sprites[1], (void*)Spr_Addr, sizeof(Raster_Sprites[1]));
	if (Raster_ColorSplit)
		memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
	memcpy(&Raster_Data[0].regs, &VDP_Reg, sizeof(VDP_Reg));
	if (Render_Point == 0)
		Flip();
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Lenght)
//    main68k_addCycles (Update_DMA ());
	M68K_AddCycles(Update_DMA ());

	if (--HInt_Counter < 0)
		{
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}

	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	
if (inkey == '6') {
	extern int Last_BUS_REQ_Cnt;
	printf("Pre Hblank:\n68k PC: %08x\n", M68K_GetPC());
	printf("68k cycles: %i\n", M68K_GetCyclesDone());
	printf("Z80 state: %08x\n", Z80_State);
	printf("Z80 last bus req: %i\n", Last_BUS_REQ_Cnt);
	printf("Z80 PC: %04x\n", z80_get_reg(Z80_REG_PC));
	fflush(stdout);
}
	
//  main68k_exec (Cycles_M68K - 360);
	M68K_Exec(Cycles_M68K - 360);

	Z80_Process(Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened

	VDP_Int |= 0x8;
	Update_IRQ_Line ();

	Z80_IRQ();

//  main68k_exec (Cycles_M68K);
	M68K_Exec(Cycles_M68K);

	Z80_Process(Cycles_Z80);
if (inkey == '6') {
	extern int Last_BUS_REQ_Cnt;
	printf("Post Hblank:\n68k PC: %08x\n", M68K_GetPC());
	printf("68k cycles: %i\n", M68K_GetCyclesDone());
	printf("Z80 state: %08x\n", Z80_State);
	printf("Z80 last bus req: %i\n", Last_BUS_REQ_Cnt);
	printf("Z80 PC: %04x\n", z80_get_reg(Z80_REG_PC));
	fflush(stdout);
}

	for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
			 VDP_Current_Line++)
		{
			buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
			buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
			YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
			YM_Len += Sound_Extrapol[VDP_Current_Line][1];
			PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

			Fix_Controllers ();
			Cycles_M68K += CPL_M68K;
			Cycles_Z80 += CPL_Z80;
			if (DMAT_Lenght)
		//main68k_addCycles (Update_DMA ());
		M68K_AddCycles(Update_DMA ());

			VDP_Status |= 0x0004;	// HBlank = 1
//              main68k_exec(Cycles_M68K - 436);
//      main68k_exec (Cycles_M68K - 404);
			M68K_Exec(Cycles_M68K - 404);

			VDP_Status &= 0xFFFB;	// HBlank = 0

//      main68k_exec (Cycles_M68K);
			M68K_Exec(Cycles_M68K);

		Z80_Process(Cycles_Z80);
		}

	PSG_Special_Update ();
	YM2612_Special_Update ();
if (!Use_SDL_Audio) {
	//~ printf("C"); fflush(stdout);
	//~ snd_stream_start(game_stream, Sound_Rate, Sound_Stereo);
	int ret = gpdc_snd_stream_poll();
	if (ret) {
		printf("SSP: %i\n", ret);fflush(stdout);
	}
}
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return (1);
}

int Do_Genesis_Frame(void)
{
	frameskipping = 0;
	prof_inc_frame_count();
	prof_start(PROF_MAINLOOP);
	int *buf[2];
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
		VDP_Num_Vis_Lines = 240;
	else
		VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
//  main68k_tripOdometer ();
	M68K_ResetCyclesCounter();

	Z80_Clear_Clock();

	Patch_Codes ();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H

	extern void memcpy_cl(void *dst, void *src, size_t len);
	extern void memsetl_cl(void *dst, int val, size_t len);
	if (0) {
		memcpy(Raster0_CRam, CRam, sizeof(Raster0_CRam));
		memset(Raster_Lines, 0, sizeof(Raster_Lines));
		memcpy(Raster_Sprites[0], (void*)Spr_Addr, sizeof(Raster_Sprites[0]));
	} else {
		
		memcpy_cl(Raster0_CRam, CRam, sizeof(Raster0_CRam)/32);
		memsetl_cl(Raster_Lines, 0, sizeof(Raster_Lines)/32);
		memcpy_cl(Raster_Sprites[0], (void*)Spr_Addr, sizeof(Raster_Sprites[0])/32);
	}
	Raster_ColorSplit = 0;
	Raster_ColorSplitLine = 1<<17;
	Raster_SpriteSplitLine = 224;

	for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
			 VDP_Current_Line++)
		{
			//~ CRam_Line_Dirty = 0;
			//~ VSRam_Line_Dirty = 0;
			//~ Raster_Lines[VDP_Current_Line] = 0;
			
			buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
			buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
			YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
			YM_Len += Sound_Extrapol[VDP_Current_Line][1];
			PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

			Fix_Controllers ();
			Cycles_M68K += CPL_M68K;
			Cycles_Z80 += CPL_Z80;
			if (DMAT_Lenght)
//				main68k_addCycles (Update_DMA ());
				M68K_AddCycles(Update_DMA ());

			VDP_Status |= 0x0004;	// HBlank = 1
//      main68k_exec (Cycles_M68K - 404);
			M68K_Exec(Cycles_M68K - 404);

			VDP_Status &= 0xFFFB;	// HBlank = 0

			if (--HInt_Counter < 0)
			{
				HInt_Counter = VDP_Reg.H_Int;
				VDP_Int |= 0x4;
				Update_IRQ_Line ();
			}

			

//      prof_start(PROF_VRENDER);
//      Render_Line ();
//      prof_end(PROF_VRENDER);
	
			M68K_Exec(Cycles_M68K);
			Z80_Process(Cycles_Z80);
		//*	
			
			
			if (1) {
				if (1) {
					float *src = (float*)&VDP_Reg;
					float *dst = &Raster_Data[VDP_Current_Line].regs;
					register int __value  __asm__("r0") = *(int*)src;
					__asm__ __volatile__ ("movca.l r0,@%0\n\t" :  : "r" (dst), "r" (__value)   );
					dst[1] = src[1];
					dst[2] = src[2];
					dst[3] = src[3];
					dst[4] = src[4];
				} else if (0) {
					float *src = (void*)&VDP_Reg;
					int len = (19 * sizeof(VDPReg_t) + 3) / 4;
					float *dst = &Raster_Data[VDP_Current_Line].regs;
					do {
						*dst++ = *src++;
					} while(--len);
				} else {
					memcpy(&Raster_Data[VDP_Current_Line].regs, &VDP_Reg, sizeof(VDP_Reg));
				}
				
				Raster_BgColor[VDP_Current_Line] = ((short*)CRam)[VDP_Reg.BG_Color];
				
				//~ if (Raster_Lines[VDP_Current_Line] & RASTER_LINE_SPRLIST_CHANGE)
					//~ Raster_SpriteSplitLine = VDP_Current_Line;
			
				//~ if ((Raster_Lines[VDP_Current_Line] & RASTER_LINE_COLOR_CHANGE) && !Raster_ColorSplit) {
					//~ Raster_ColorSplit = 1;
					//~ Raster_ColorSplitLine = VDP_Current_Line;
					//~ //memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
				//~ }
				
				if (!(VDP_Reg.Set2 & 0x40)) {	//Is screen off?
					Raster_Lines[VDP_Current_Line] |= RASTER_LINE_SCREEN_OFF;
				}
			
				// else
				//~ if (VDP_Reg.Set3 & 0x4) {
					//~ //Read from center of screen when vert cell scroll is on
					//~ Raster_Data[VDP_Current_Line].vscroll_a = ((short*)&VSRam)[20];
					//~ Raster_Data[VDP_Current_Line].vscroll_b = ((short*)&VSRam)[21];
				//~ } else {
					extern u32 V_Scroll_CMask;
					//~ int vmask = (V_Scroll_CMask << 3) | 0x7;
					int vmask = 0x3ff;
					Raster_Data[VDP_Current_Line].vscroll_a = ((short*)&VSRam)[0]&vmask;
					Raster_Data[VDP_Current_Line].vscroll_b = ((short*)&VSRam)[1]&vmask;
					
				//~ }
				extern int H_Scroll_Mask;
				extern unsigned int H_Scroll_Addr;
				short *hscrolltable = (short*)H_Scroll_Addr;
				Raster_Data[VDP_Current_Line].hscroll_a = hscrolltable[(VDP_Current_Line&H_Scroll_Mask)*2];
				Raster_Data[VDP_Current_Line].hscroll_b = hscrolltable[(VDP_Current_Line&H_Scroll_Mask)*2+1];
				//~ Raster_Data[VDP_Current_Line].hscroll_b = ((short*)&VSRam)[1];
				
			}
			if (Render_Point == 2 && (Raster_Lines[VDP_Current_Line] & RASTER_LINE_SCREEN_OFF)) {
				Render_Point = 3;
				int i;
				for(i = VDP_Current_Line; i < (VDP_Num_Vis_Lines-1); i++) {
					memcpy(Raster_Lines + i + 1, Raster_Lines + i, sizeof(*Raster_Lines));
				}
				Flip ();
			}
			//*/
			
		}
	
	if (Raster_SpriteSplitLine < 224)
		//~ memcpy(Raster_Sprites[1], (void*)Spr_Addr, sizeof(Raster_Sprites[1]));
		memcpy_cl(Raster_Sprites[1], (void*)Spr_Addr, sizeof(Raster_Sprites[1])/32);
	if (Raster_ColorSplit) {
		//~ printf("S"); fflush(stdout);
		//~ memcpy(Raster1_CRam, CRam, sizeof(Raster1_CRam));
		memcpy_cl(Raster1_CRam, CRam, sizeof(Raster1_CRam)/32);
	}
	if (Render_Point == 0 || Render_Point == 2)
		Flip();
	if (Render_Point == 3)
		Render_Point = 2;
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Lenght)
//    main68k_addCycles (Update_DMA ());
	M68K_AddCycles(Update_DMA ());

	if (--HInt_Counter < 0)
		{
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}

	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	
if (inkey == '6') {
	extern int Last_BUS_REQ_Cnt;
	printf("Pre Hblank:\n68k PC: %08x\n", M68K_GetPC());
	printf("68k cycles: %i\n", M68K_GetCyclesDone());
	printf("Z80 state: %08x\n", Z80_State);
	printf("Z80 last bus req: %i\n", Last_BUS_REQ_Cnt);
	printf("Z80 PC: %04x\n", z80_get_reg(Z80_REG_PC));
	fflush(stdout);
}
	
//  main68k_exec (Cycles_M68K - 360);
	M68K_Exec(Cycles_M68K - 360);

	Z80_Process(Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened

	VDP_Int |= 0x8;
	Update_IRQ_Line ();

	Z80_IRQ();

//  main68k_exec (Cycles_M68K);
	M68K_Exec(Cycles_M68K);

	Z80_Process(Cycles_Z80);
if (0 && inkey == '6') {
	extern int Last_BUS_REQ_Cnt;
	printf("Post Hblank:\n68k PC: %08x\n", M68K_GetPC());
	printf("68k cycles: %i\n", M68K_GetCyclesDone());
	printf("Z80 state: %08x\n", Z80_State);
	printf("Z80 last bus req: %i\n", Last_BUS_REQ_Cnt);
	printf("Z80 PC: %04x\n", z80_get_reg(Z80_REG_PC));
	fflush(stdout);
}

	for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
			 VDP_Current_Line++)
		{
			buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
			buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
			YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
			YM_Len += Sound_Extrapol[VDP_Current_Line][1];
			PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

			Fix_Controllers ();
			Cycles_M68K += CPL_M68K;
			Cycles_Z80 += CPL_Z80;
			if (DMAT_Lenght)
		//main68k_addCycles (Update_DMA ());
		M68K_AddCycles(Update_DMA ());

			VDP_Status |= 0x0004;	// HBlank = 1
//              main68k_exec(Cycles_M68K - 436);
//      main68k_exec (Cycles_M68K - 404);
			M68K_Exec(Cycles_M68K - 404);

			VDP_Status &= 0xFFFB;	// HBlank = 0

//      main68k_exec (Cycles_M68K);
			M68K_Exec(Cycles_M68K);

		Z80_Process(Cycles_Z80);
		}

	PSG_Special_Update ();
	YM2612_Special_Update ();
#if 0
if (!Use_SDL_Audio) {
	//~ printf("C"); fflush(stdout);
	//~ snd_stream_start(game_stream, Sound_Rate, Sound_Stereo);
	int ret = gpdc_snd_stream_poll();
	if (ret) {
		printf("SSP: %i\n", ret);fflush(stdout);
	}
}
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
#endif

	prof_end(PROF_MAINLOOP);

	return (1);
}

#ifdef DEBUG_GENESIS
int Do_VDP_Only(void)
{
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
		VDP_Num_Vis_Lines = 240;
	else
		VDP_Num_Vis_Lines = 224;
	
	for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
		Render_Line();

	return (0);
}
#endif


int Do_Genesis_Frame_No_VDP (void)
{
	frameskipping = 1;
	
	prof_inc_frame_count();
  int *buf[2];
  int HInt_Counter;

  if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
    VDP_Num_Vis_Lines = 240;
  else
    VDP_Num_Vis_Lines = 224;

  YM_Buf[0] = PSG_Buf[0] = Seg_L;
  YM_Buf[1] = PSG_Buf[1] = Seg_R;
  YM_Len = PSG_Len = 0;

  Cycles_M68K = Cycles_Z80 = 0;
  Last_BUS_REQ_Cnt = -1000;
//  main68k_tripOdometer ();
	M68K_ResetCyclesCounter();

	Z80_Clear_Clock();

  Patch_Codes ();

  VRam_Flag = 1;

  VDP_Status &= 0xFFF7;		// Clear V Blank
  if (VDP_Reg.Set4 & 0x2)
    VDP_Status ^= 0x0010;

  HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt

  for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (DMAT_Lenght)
//	main68k_addCycles (Update_DMA ());
		M68K_AddCycles(Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
//              main68k_exec(Cycles_M68K - 436);
//      main68k_exec (Cycles_M68K - 404);
		M68K_Exec(Cycles_M68K - 404);

      VDP_Status &= 0xFFFB;	// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}

//		main68k_exec (Cycles_M68K);
		M68K_Exec (Cycles_M68K);

		Z80_Process(Cycles_Z80);
    }

  buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
  buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
  YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM_Len += Sound_Extrapol[VDP_Current_Line][1];
  PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

  Fix_Controllers ();
  Cycles_M68K += CPL_M68K;
  Cycles_Z80 += CPL_Z80;
  if (DMAT_Lenght)
//    main68k_addCycles (Update_DMA ());
	M68K_AddCycles(Update_DMA());

  if (--HInt_Counter < 0)
    {
      VDP_Int |= 0x4;
      Update_IRQ_Line ();
    }

  VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
//  main68k_exec (Cycles_M68K - 360);
	M68K_Exec(Cycles_M68K - 360);

	Z80_Process(Cycles_Z80 - 168);

  VDP_Status &= 0xFFFB;		// HBlank = 0
  VDP_Status |= 0x0080;		// V Int happened
  VDP_Int |= 0x8;
  Update_IRQ_Line ();

	Z80_IRQ();

//  main68k_exec (Cycles_M68K);
	M68K_Exec(Cycles_M68K);

	Z80_Process(Cycles_Z80);

  for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (DMAT_Lenght)
//	main68k_addCycles (Update_DMA ());
		M68K_AddCycles(Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
//      main68k_exec (Cycles_M68K - 404);
		M68K_Exec(Cycles_M68K - 404);

      VDP_Status &= 0xFFFB;	// HBlank = 0

//      main68k_exec (Cycles_M68K);
		M68K_Exec(Cycles_M68K);

		Z80_Process(Cycles_Z80);
    }

  PSG_Special_Update ();
  YM2612_Special_Update ();

  if (WAV_Dumping)
    Update_WAV_Dump ();
  if (GYM_Dumping)
    Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

  return (1);
}
