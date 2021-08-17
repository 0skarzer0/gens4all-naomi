/****************************************************************************/
/* SEGA CD main module                                                      */
/****************************************************************************/

#include "segacd.h"


int Init_SegaCD (char *iso_name)
{
  char Str_Err[256], *Bios_To_Use;

  SetWindowText ("Gens - Sega CD : initialising, please wait ...");

  if (Reset_CD ((char *) CD_Data, iso_name))
    {
      SetWindowText ("Gens - Idle");
      return 0;
    }

  switch (Country)
    {
    default:
    case -1:
      Bios_To_Use = Detect_Country_SegaCD ();
      break;

    case 0:
      Game_Mode = 0;
      CPU_Mode = 0;
      Bios_To_Use = JA_CD_Bios;
      break;

    case 1:
      Game_Mode = 1;
      CPU_Mode = 0;
      Bios_To_Use = US_CD_Bios;
      break;

    case 2:
      Game_Mode = 1;
      CPU_Mode = 1;
      Bios_To_Use = EU_CD_Bios;
      break;

    case 3:
      Game_Mode = 0;
      CPU_Mode = 1;
      Bios_To_Use = JA_CD_Bios;
      break;
    }

	if (Load_Bios (Bios_To_Use) == NULL)
	{
		/* the BIOS files are not correctly configured */
		SetWindowText ("Gens - Idle");
		return 0;
	}

  Update_CD_Rom_Name ((char *) &CD_Data[32]);

  if ((CPU_Mode == 1) || (Game_Mode == 0))
    sprintf (Str_Err, "Gens - MegaCD : %s", Rom_Name);
  else
    sprintf (Str_Err, "Gens - SegaCD : %s", Rom_Name);

  SetWindowText (Str_Err);

  Flag_Clr_Scr = 1;
  Debug = Paused = Frame_Number = 0;
  SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
  BRAM_Ex_State &= 0x100;
  Controller_1_COM = Controller_2_COM = 0;

  if (CPU_Mode)
    {
      CPL_Z80 = Round_Double ((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
      CPL_M68K = Round_Double ((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);

#ifdef SEGA_32X
      CPL_MSH2 =
	Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
		       (double) MSH2_Speed) / 100.0);
      CPL_SSH2 =
	Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
		       (double) SSH2_Speed) / 100.0);
#endif

      VDP_Num_Lines = 312;
      VDP_Status |= 0x0001;

      CD_Access_Timer = 2080;
//      Timer_Step = 136752;
    }
  else
    {
      CPL_Z80 = Round_Double ((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
      CPL_M68K = Round_Double ((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);

#ifdef SEGA_32X
      CPL_MSH2 =
	Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) /
			262.0) * (double) MSH2_Speed) / 100.0);
      CPL_SSH2 =
	Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) /
			262.0) * (double) SSH2_Speed) / 100.0);
#endif

      VDP_Num_Lines = 262;
      VDP_Status &= 0xFFFE;

      CD_Access_Timer = 2096;
//      Timer_Step = 135708;
    }

  VDP_Num_Vis_Lines = 224;
  Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)

  Rom_Data[0x72] = 0xFF;
  Rom_Data[0x73] = 0xFF;
  Byte_Swap (Rom_Data, Rom_Size);

  M68K_Reset (2);
  S68K_Reset ();
  Z80_Reset ();
  Reset_VDP ();
  LC89510_Reset ();
  Init_RS_GFX ();

  if (CPU_Mode)
    {
      YM2612_Init (CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
      PSG_Init (CLOCK_PAL / 15, Sound_Rate);
    }
  else
    {
      YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
      PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
    }

  Init_PCM (Sound_Rate);

  if (Sound_Enable)
    {
      End_Sound ();

      if (!Init_Sound ())
	Sound_Enable = 0;
      else
	Play_Sound ();
    }

	Load_BRAM ();			// Initialise BRAM
	Load_Patch_File ();		// Only used to reset Patch structure
	
	Last_Time = GetTickCount ();
	New_Time = 0;
	Used_Time = 0;

	if (SegaCD_Accurate)
	{
		Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}
	else
	{
		Update_Frame = Do_SegaCD_Frame;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}

	return 1;
}


int Reload_SegaCD (char *iso_name)
{
  char Str_Err[256];

  Save_BRAM ();

  SetWindowText ("Gens - Sega CD : re-initialising, please wait ...");

  Reset_CD ((char *) CD_Data, iso_name);
  Update_CD_Rom_Name ((char *) &CD_Data[32]);

  if ((CPU_Mode == 1) || (Game_Mode == 0))
    sprintf (Str_Err, "Gens - MegaCD : %s", Rom_Name);
  else
    sprintf (Str_Err, "Gens - SegaCD : %s", Rom_Name);

  SetWindowText (Str_Err);

  Load_BRAM ();

  return 1;
}


void Reset_SegaCD(void)
{
  char *Bios_To_Use;

  if (CPU_Mode)
    Bios_To_Use = EU_CD_Bios;
  else if (Game_Mode)
    Bios_To_Use = US_CD_Bios;
  else
    Bios_To_Use = JA_CD_Bios;

  //SetCurrentDirectory(Gens_Path);

  if (Detect_Format (Bios_To_Use) == -1)
    {
      puts
	("Some bios files are missing !\nConfigure them in the option menu.\n");
      //MessageBox(HWnd, "Some bios files are missing !\nConfigure them in the option menu.\n", "Warning", MB_ICONEXCLAMATION | MB_OK);
      return;
    }

  Controller_1_COM = Controller_2_COM = 0;
  SRAM_ON = 0;
  SRAM_Write = 0;
  Paused = 0;
  BRAM_Ex_State &= 0x100;

  if (!stricmp ("ZIP", &Bios_To_Use[strlen (Bios_To_Use) - 3]))
    {
      Game = Load_Rom_Zipped (Bios_To_Use, 0);
    }
  else
    {
      Game = Load_Rom (Bios_To_Use, 0);
    }

  Update_CD_Rom_Name ((char *) &CD_Data[32]);

  Rom_Data[0x72] = 0xFF;
  Rom_Data[0x73] = 0xFF;

  Byte_Swap (Rom_Data, Rom_Size);

  M68K_Reset (2);
  S68K_Reset ();
  Z80_Reset ();
  LC89510_Reset ();
  Reset_VDP ();
  Init_RS_GFX ();
  Reset_PCM ();
  YM2612_Reset ();

  if (CPU_Mode)
    VDP_Status |= 1;
  else
    VDP_Status &= ~1;
}


int Do_SegaCD_Frame_No_VDP (void)
{
  int *buf[2];
  int HInt_Counter;

  if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
    VDP_Num_Vis_Lines = 240;
  else
    VDP_Num_Vis_Lines = 224;

  CPL_S68K = 795;

  YM_Buf[0] = PSG_Buf[0] = Seg_L;
  YM_Buf[1] = PSG_Buf[1] = Seg_R;
  YM_Len = PSG_Len = 0;

  Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
  Last_BUS_REQ_Cnt = -1000;
  main68k_tripOdometer ();
  sub68k_tripOdometer ();

	Z80_Clear_Clock();
//  z80_Clear_Odo (&M_Z80);

  VRam_Flag = 1;

  VDP_Status &= 0xFFF7;
  if (VDP_Reg.Set4 & 0x2)
    VDP_Status ^= 0x0010;

  HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt

  for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
      main68k_exec (Cycles_M68K - 404);
      VDP_Status &= 0xFFFB;	// HBlank = 0

      if (--HInt_Counter < 0)
	{
	  HInt_Counter = VDP_Reg.H_Int;
	  VDP_Int |= 0x4;
	  Update_IRQ_Line ();
	}

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);
		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
  buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
  if (PCM_Enable)
    Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM_Len += Sound_Extrapol[VDP_Current_Line][1];
  PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
  Update_CDC_TRansfert ();

  Fix_Controllers ();
  Cycles_M68K += CPL_M68K;
  Cycles_Z80 += CPL_Z80;
  if (S68K_State == 1)
    Cycles_S68K += CPL_S68K;
  if (DMAT_Lenght)
    main68k_addCycles (Update_DMA ());

  if (--HInt_Counter < 0)
    {
      VDP_Int |= 0x4;
      Update_IRQ_Line ();
    }

  VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
  main68k_exec (Cycles_M68K - 360);
  sub68k_exec (Cycles_S68K - 586);
	Z80_Process(Cycles_Z80 - 168);
  VDP_Status &= 0xFFFB;		// HBlank = 0
  VDP_Status |= 0x0080;		// V Int happened
  VDP_Int |= 0x8;
  Update_IRQ_Line ();

	Z80_IRQ();

  main68k_exec (Cycles_M68K);
  sub68k_exec (Cycles_S68K);

	Z80_Process(Cycles_Z80);
  Update_SegaCD_Timer ();

  for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
      main68k_exec (Cycles_M68K - 404);
      VDP_Status &= 0xFFFB;	// HBlank = 0

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L;
  buf[1] = Seg_R;

  PSG_Special_Update ();
  YM2612_Special_Update ();
  Update_CD_Audio (buf, Seg_Lenght);

  if (WAV_Dumping)
    Update_WAV_Dump ();
  if (GYM_Dumping)
    Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

  return (1);
}


int Do_SegaCD_Frame_No_VDP_Cycle_Accurate (void)
{
  int *buf[2], i, j;
  int HInt_Counter;

  if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
    VDP_Num_Vis_Lines = 240;
  else
    VDP_Num_Vis_Lines = 224;

  CPL_S68K = 795;

  YM_Buf[0] = PSG_Buf[0] = Seg_L;
  YM_Buf[1] = PSG_Buf[1] = Seg_R;
  YM_Len = PSG_Len = 0;

  Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
  Last_BUS_REQ_Cnt = -1000;
  main68k_tripOdometer ();
  sub68k_tripOdometer ();

	Z80_Clear_Clock();

  VRam_Flag = 1;

  VDP_Status &= 0xFFF7;
  if (VDP_Reg.Set4 & 0x2)
    VDP_Status ^= 0x0010;

  HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt

  for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      i = Cycles_M68K + 24;
      j = Cycles_S68K + 39;

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1

      /* instruction by instruction execution */

      while (i < (Cycles_M68K - 404))
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < (Cycles_S68K - 658))
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K - 404);
      sub68k_exec (Cycles_S68K - 658);

      /* end instruction by instruction execution */

      VDP_Status &= 0xFFFB;	// HBlank = 0

      if (--HInt_Counter < 0)
	{
	  HInt_Counter = VDP_Reg.H_Int;
	  VDP_Int |= 0x4;
	  Update_IRQ_Line ();
	}

      /* instruction by instruction execution */

      while (i < Cycles_M68K)
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < Cycles_S68K)
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

      /* end instruction by instruction execution */
		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
  buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
  if (PCM_Enable)
    Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM_Len += Sound_Extrapol[VDP_Current_Line][1];
  PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
  Update_CDC_TRansfert ();

  i = Cycles_M68K + 24;
  j = Cycles_S68K + 39;

  Fix_Controllers ();
  Cycles_M68K += CPL_M68K;
  Cycles_Z80 += CPL_Z80;
  if (S68K_State == 1)
    Cycles_S68K += CPL_S68K;
  if (DMAT_Lenght)
    main68k_addCycles (Update_DMA ());

  if (--HInt_Counter < 0)
    {
      VDP_Int |= 0x4;
      Update_IRQ_Line ();
    }

  VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)

  /* instruction by instruction execution */

  while (i < (Cycles_M68K - 360))
    {
      main68k_exec (i);
      i += 24;

      if (j < (Cycles_S68K - 586))
	{
	  sub68k_exec (j);
	  j += 39;
	}
    }

  main68k_exec (Cycles_M68K - 360);
  sub68k_exec (Cycles_S68K - 586);

  /* end instruction by instruction execution */

	Z80_Process(Cycles_Z80 - 168);
  VDP_Status &= 0xFFFB;		// HBlank = 0
  VDP_Status |= 0x0080;		// V Int happened
  VDP_Int |= 0x8;
  Update_IRQ_Line ();

	Z80_IRQ();

  /* instruction by instruction execution */

  while (i < Cycles_M68K)
    {
      main68k_exec (i);
      i += 24;

      if (j < Cycles_S68K)
	{
	  sub68k_exec (j);
	  j += 39;
	}
    }

  main68k_exec (Cycles_M68K);
  sub68k_exec (Cycles_S68K);

  /* end instruction by instruction execution */
	Z80_Process(Cycles_Z80);
  Update_SegaCD_Timer ();

  for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      i = Cycles_M68K + 24;
      j = Cycles_S68K + 39;

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1

      /* instruction by instruction execution */

      while (i < (Cycles_M68K - 404))
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < (Cycles_S68K - 658))
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K - 404);
      sub68k_exec (Cycles_S68K - 658);

      /* end instruction by instruction execution */

      VDP_Status &= 0xFFFB;	// HBlank = 0

      /* instruction by instruction execution */

      while (i < Cycles_M68K)
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < Cycles_S68K)
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

      /* end instruction by instruction execution */
		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L;
  buf[1] = Seg_R;

  PSG_Special_Update ();
  YM2612_Special_Update ();
  Update_CD_Audio (buf, Seg_Lenght);

  if (WAV_Dumping)
    Update_WAV_Dump ();
  if (GYM_Dumping)
    Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

  return (1);
}


int Do_SegaCD_Frame (void)
{
  int *buf[2];
  int HInt_Counter;

  if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
    VDP_Num_Vis_Lines = 240;
  else
    VDP_Num_Vis_Lines = 224;

  CPL_S68K = 795;

  YM_Buf[0] = PSG_Buf[0] = Seg_L;
  YM_Buf[1] = PSG_Buf[1] = Seg_R;
  YM_Len = PSG_Len = 0;

  Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
  Last_BUS_REQ_Cnt = -1000;
  main68k_tripOdometer ();
  sub68k_tripOdometer ();

	Z80_Clear_Clock();

  VRam_Flag = 1;

  VDP_Status &= 0xFFF7;
  if (VDP_Reg.Set4 & 0x2)
    VDP_Status ^= 0x0010;

  HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H

  for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
      main68k_exec (Cycles_M68K - 404);
      VDP_Status &= 0xFFFB;	// HBlank = 0

      if (--HInt_Counter < 0)
	{
	  HInt_Counter = VDP_Reg.H_Int;
	  VDP_Int |= 0x4;
	  Update_IRQ_Line ();
	}

      Render_Line ();

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
  buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
  if (PCM_Enable)
    Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM_Len += Sound_Extrapol[VDP_Current_Line][1];
  PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
  Update_CDC_TRansfert ();

  Fix_Controllers ();
  Cycles_M68K += CPL_M68K;
  Cycles_Z80 += CPL_Z80;
  if (S68K_State == 1)
    Cycles_S68K += CPL_S68K;
  if (DMAT_Lenght)
    main68k_addCycles (Update_DMA ());

  if (--HInt_Counter < 0)
    {
      VDP_Int |= 0x4;
      Update_IRQ_Line ();
    }

  VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
  main68k_exec (Cycles_M68K - 360);
  sub68k_exec (Cycles_S68K - 586);
	Z80_Process(Cycles_Z80 - 168);

  VDP_Status &= 0xFFFB;		// HBlank = 0
  VDP_Status |= 0x0080;		// V Int happened
  VDP_Int |= 0x8;
  Update_IRQ_Line ();

	Z80_IRQ();

  main68k_exec (Cycles_M68K);
  sub68k_exec (Cycles_S68K);

	Z80_Process(Cycles_Z80);
  Update_SegaCD_Timer ();

  for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1
      main68k_exec (Cycles_M68K - 404);
      VDP_Status &= 0xFFFB;	// HBlank = 0

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

		Z80_Process(Cycles_Z80);

      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L;
  buf[1] = Seg_R;

  PSG_Special_Update ();
  YM2612_Special_Update ();
  Update_CD_Audio (buf, Seg_Lenght);

  if (WAV_Dumping)
    Update_WAV_Dump ();
  if (GYM_Dumping)
    Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

  if (Show_LED)
    {
      if (LED_Status & 2)
	{
	  MD_Screen[336 * 220 + 12] = 0x03E0;
	  MD_Screen[336 * 220 + 13] = 0x03E0;
	  MD_Screen[336 * 220 + 14] = 0x03E0;
	  MD_Screen[336 * 220 + 15] = 0x03E0;
	  MD_Screen[336 * 222 + 12] = 0x03E0;
	  MD_Screen[336 * 222 + 13] = 0x03E0;
	  MD_Screen[336 * 222 + 14] = 0x03E0;
	  MD_Screen[336 * 222 + 15] = 0x03E0;
	}

      if (LED_Status & 1)
	{
	  MD_Screen[336 * 220 + 12 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 13 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 14 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 15 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 12 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 13 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 14 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 15 + 8] = 0xF800;
	}
    }

  return (1);
}

int Do_SegaCD_Frame_Cycle_Accurate (void)
{
  int *buf[2], i, j;
  int HInt_Counter;

  if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
    VDP_Num_Vis_Lines = 240;
  else
    VDP_Num_Vis_Lines = 224;

  CPL_S68K = 795;

  YM_Buf[0] = PSG_Buf[0] = Seg_L;
  YM_Buf[1] = PSG_Buf[1] = Seg_R;
  YM_Len = PSG_Len = 0;

  Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
  Last_BUS_REQ_Cnt = -1000;
  main68k_tripOdometer ();
  sub68k_tripOdometer ();

	Z80_Clear_Clock();

  VRam_Flag = 1;

  VDP_Status &= 0xFFF7;
  if (VDP_Reg.Set4 & 0x2)
    VDP_Status ^= 0x0010;

  HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H

  for (VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      i = Cycles_M68K + 24;
      j = Cycles_S68K + 39;

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1

      /* instruction by instruction execution */

      while (i < (Cycles_M68K - 404))
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < (Cycles_S68K - 658))
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K - 404);
      sub68k_exec (Cycles_S68K - 658);

      /* end instruction by instruction execution */

      VDP_Status &= 0xFFFB;	// HBlank = 0

      if (--HInt_Counter < 0)
	{
	  HInt_Counter = VDP_Reg.H_Int;
	  VDP_Int |= 0x4;
	  Update_IRQ_Line ();
	}

      Render_Line ();

      /* instruction by instruction execution */

      while (i < Cycles_M68K)
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < Cycles_S68K)
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

      /* end instruction by instruction execution */
		Z80_Process(Cycles_Z80);
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
  buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
  if (PCM_Enable)
    Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
  YM_Len += Sound_Extrapol[VDP_Current_Line][1];
  PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
  Update_CDC_TRansfert ();

  i = Cycles_M68K + 24;
  j = Cycles_S68K + 39;

  Fix_Controllers ();
  Cycles_M68K += CPL_M68K;
  Cycles_Z80 += CPL_Z80;
  if (S68K_State == 1)
    Cycles_S68K += CPL_S68K;
  if (DMAT_Lenght)
    main68k_addCycles (Update_DMA ());

  if (--HInt_Counter < 0)
    {
      VDP_Int |= 0x4;
      Update_IRQ_Line ();
    }

  VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)

  /* instruction by instruction execution */

  while (i < (Cycles_M68K - 360))
    {
      main68k_exec (i);
      i += 24;

      if (j < (Cycles_S68K - 586))
	{
	  sub68k_exec (j);
	  j += 39;
	}
    }

  main68k_exec (Cycles_M68K - 360);
  sub68k_exec (Cycles_S68K - 586);

  /* end instruction by instruction execution */
	Z80_Process(Cycles_Z80 - 168);


  VDP_Status &= 0xFFFB;		// HBlank = 0
  VDP_Status |= 0x0080;		// V Int happened
  VDP_Int |= 0x8;
  Update_IRQ_Line ();

	Z80_IRQ();

  /* instruction by instruction execution */

  while (i < Cycles_M68K)
    {
      main68k_exec (i);
      i += 24;

      if (j < Cycles_S68K)
	{
	  sub68k_exec (j);
	  j += 39;
	}
    }

  main68k_exec (Cycles_M68K);
  sub68k_exec (Cycles_S68K);

  /* end instruction by instruction execution */
	Z80_Process(Cycles_Z80);
  Update_SegaCD_Timer ();

  for (VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines;
       VDP_Current_Line++)
    {
      buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
      buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
      if (PCM_Enable)
	Update_PCM (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
      YM_Len += Sound_Extrapol[VDP_Current_Line][1];
      PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
      Update_CDC_TRansfert ();

      i = Cycles_M68K + 24;
      j = Cycles_S68K + 39;

      Fix_Controllers ();
      Cycles_M68K += CPL_M68K;
      Cycles_Z80 += CPL_Z80;
      if (S68K_State == 1)
	Cycles_S68K += CPL_S68K;
      if (DMAT_Lenght)
	main68k_addCycles (Update_DMA ());

      VDP_Status |= 0x0004;	// HBlank = 1

      /* instruction by instruction execution */

      while (i < (Cycles_M68K - 404))
	{
	  main68k_exec (i);
	  i += 24;

	  if (j < (Cycles_S68K - 658))
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K - 404);
      sub68k_exec (Cycles_S68K - 658);

      /* end instruction by instruction execution */

      VDP_Status &= 0xFFFB;	// HBlank = 0

      /* instruction by instruction execution */

      while (i < Cycles_M68K)
	{
	  main68k_exec (i);
	  i += 24;		// Chuck Rock intro need faster timing ... strange.

	  if (j < Cycles_S68K)
	    {
	      sub68k_exec (j);
	      j += 39;
	    }
	}

      main68k_exec (Cycles_M68K);
      sub68k_exec (Cycles_S68K);

	Z80_Process(Cycles_Z80);
      /* end instruction by instruction execution */
      Update_SegaCD_Timer ();
    }

  buf[0] = Seg_L;
  buf[1] = Seg_R;

  PSG_Special_Update ();
  YM2612_Special_Update ();
  Update_CD_Audio (buf, Seg_Lenght);

  if (WAV_Dumping)
    Update_WAV_Dump ();
  if (GYM_Dumping)
    Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

  if (Show_LED)
    {
      if (LED_Status & 2)
	{
	  MD_Screen[336 * 220 + 12] = 0x03E0;
	  MD_Screen[336 * 220 + 13] = 0x03E0;
	  MD_Screen[336 * 220 + 14] = 0x03E0;
	  MD_Screen[336 * 220 + 15] = 0x03E0;
	  MD_Screen[336 * 222 + 12] = 0x03E0;
	  MD_Screen[336 * 222 + 13] = 0x03E0;
	  MD_Screen[336 * 222 + 14] = 0x03E0;
	  MD_Screen[336 * 222 + 15] = 0x03E0;
	}

      if (LED_Status & 1)
	{
	  MD_Screen[336 * 220 + 12 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 13 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 14 + 8] = 0xF800;
	  MD_Screen[336 * 220 + 15 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 12 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 13 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 14 + 8] = 0xF800;
	  MD_Screen[336 * 222 + 15 + 8] = 0xF800;
	}
    }

  return 1;
}
