
#include "gens4all.h"
#include "misc.h"

_filters filters;

void
save_state ()
{
  Str_Tmp[0] = 0;
  Get_State_File_Name (Str_Tmp);
  Save_State (Str_Tmp);
}

void
load_state ()
{
  Str_Tmp[0] = 0;
  Get_State_File_Name (Str_Tmp);
  Load_State (Str_Tmp);
}

void system_reset ()
{
	if (Genesis_Started)
	{
		Reset_Genesis ();
	}
#ifdef SEGA_32X
	else if (_32X_Started)
	{
		Reset_32X ();
	}
#endif
#ifdef SEGA_CD
	else if (SegaCD_Started)
	{
		Reset_SegaCD ();
	}
#endif
}

int Set_Render(int Full, int Num, int Force)
{
#ifndef DREAMCAST
  int Old_Rend, *Rend;
  void (**Blit) (unsigned char *, const int, const int, const int, const int);

  if (Full)
    {
      Rend = &Render_FS;
      Blit = &Blit_FS;
    }
  else
    {
      Rend = &Render_W;
      Blit = &Blit_W;
    }

  Old_Rend = *Rend;
  Flag_Clr_Scr = 1;

  switch (Num)
    {
    case -1:
      switch (Old_Rend)
	{
	case NORMAL:
		
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_X1_MMX;
	  else
#endif
	    *Blit = Blit_X1;
	  break;

	case DOUBLE:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_X2_MMX;
	  else
	    *Blit = Blit_X2;
#endif
	  break;

	case INTERPOLATED:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_X2_Int_MMX;
	  else
	    *Blit = Blit_X2_Int;
#endif
	  break;

	case FULL_SCANLINE:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_MMX;
	  else
	    *Blit = Blit_Scanline;
#endif
	  break;

	case SCANLINE_50:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_50_MMX;
	  else
	    {
	      *Rend = 1;
	      *Blit = Blit_X2;
	    }
#endif
	  break;

	case SCANLINE_25:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_25_MMX;
	  else
	    {
	      *Rend = 1;
	      *Blit = Blit_X2;
	    }
#endif
	  break;

	case INTERPOLATED_SCANLINE:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_Int_MMX;
	  else
	    *Blit = Blit_Scanline_Int;
#endif
	  break;

	case INTERPOLATED_SCANLINE_50:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_50_Int_MMX;
	  else
	    {
	      *Rend = 1;
	      *Blit = Blit_X2;
	    }
#endif
	  break;

	case INTERPOLATED_SCANLINE_25:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_Scanline_25_Int_MMX;
	  else
	    {
	      *Rend = 1;
	      *Blit = Blit_X2;
	    }
#endif
	  break;

	case KREED:
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_2xSAI_MMX;
	  else
	    {
	      *Rend = 1;
	      *Blit = Blit_X2;
	    }
#endif
	  break;
	case SCALE2X:
#if (TARGET_SYS,x86)
	  *Blit = Blit_Scale2x;
#endif
	  break;

	case HQ2X:
#if (TARGET_SYS,x86)
	  *Blit = _Blit_HQ2x;
#endif
	  break;

	default:
	  *Rend = 1;
#if (TARGET_SYS,x86)
	  if (Have_MMX)
	    *Blit = Blit_X2_MMX;
	  else
	    *Blit = Blit_X2;
#endif
	  break;
	}
      break;

    case NORMAL:
      *Rend = NORMAL;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_X1_MMX;
      else
	*Blit = Blit_X1;
#endif
      MESSAGE_L ("Render selected : NORMAL", "Render selected : NORMAL", 1500)
	break;

    case DOUBLE:
      *Rend = DOUBLE;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_X2_MMX;
      else
	*Blit = Blit_X2;
#endif
      MESSAGE_L ("Render selected : DOUBLE", "Render selected : DOUBLE", 1500)
	break;

    case INTERPOLATED:
      *Rend = INTERPOLATED;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_X2_Int_MMX;
      else
	*Blit = Blit_X2_Int;
#endif
      MESSAGE_L ("Render selected : INTERPOLATED",
		 "Render selected : INTERPOLATED", 1500) break;

    case FULL_SCANLINE:
      *Rend = FULL_SCANLINE;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_Scanline_MMX;
      else
	*Blit = Blit_Scanline;
#endif
      MESSAGE_L ("Render selected : FULL SCANLINE",
		 "Render selected : FULL SCANLINE", 1500) break;

    case SCANLINE_50:
#if (TARGET_SYS,x86)
      if (Have_MMX)
	{
	SELECT_RENDERER (SCANLINE_50, Blit_Scanline_50_MMX, "50% SCANLINE")}
      else
	{
	SELECT_RENDERER (INTERPOLATED_SCANLINE, Blit_Scanline_Int,
			   "INTERPOLATED SCANLINE")
	}
#endif
      break;

    case SCANLINE_25:
#if (TARGET_SYS,x86)
      if (Have_MMX)
	{
	SELECT_RENDERER (SCANLINE_25, Blit_Scanline_25_MMX, "25% SCANLINE")}
      else
	{
	SELECT_RENDERER (FULL_SCANLINE, Blit_Scanline, "FULL SCANLINE")
	}
#endif
      break;

    case INTERPOLATED_SCANLINE:
      *Rend = INTERPOLATED_SCANLINE;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_Scanline_Int_MMX;
      else
	*Blit = Blit_Scanline_Int;
#endif
      MESSAGE_L ("Render selected : INTERPOLATED SCANLINE",
		 "Render selected : INTERPOLATED SCANLINE", 1500) break;

    case INTERPOLATED_SCANLINE_50:
#if (TARGET_SYS,x86)
      if (Have_MMX)
	{
	SELECT_RENDERER (INTERPOLATED_SCANLINE_50, Blit_Scanline_50_Int_MMX,
			   "INTERPOLATED 50% SCANLINE")}
      else
	{
	SELECT_RENDERER (INTERPOLATED_SCANLINE, Blit_Scanline_Int,
			   "INTERPOLATED SCANLINE")}
#endif
      break;

    case INTERPOLATED_SCANLINE_25:
#if (TARGET_SYS,x86)
      if (Have_MMX)
	SELECT_RENDERER (INTERPOLATED_SCANLINE_25, Blit_Scanline_25_Int_MMX,
			 "INTERPOLATED 25% SCANLINE")
	else
	SELECT_RENDERER (INTERPOLATED_SCANLINE, Blit_Scanline_Int,
			 "INTERPOLATED SCANLINE")
#endif
	break;
    
	case KREED:
#if (TARGET_SYS,x86)
      if (Have_MMX)
	{
	SELECT_RENDERER (KREED, Blit_2xSAI_MMX, "2XSAI KREED'S ENGINE")}
      else
	{
	SELECT_RENDERER (INTERPOLATED_SCANLINE, Blit_Scanline_Int,
			   "INTERPOLATED SCANLINE")
	}
#endif
      break;

    case SCALE2X:
#if (TARGET_SYS,x86)
      *Rend = SCALE2X;
      *Blit = Blit_Scale2x;
      MESSAGE_L ("Render selected : AdvanceMAME Scale2x",
		 "Render selected : AdvanceMAME Scale2x", 1500)
endif
	break;

    case HQ2X:
      *Rend = HQ2X;
      *Blit = _Blit_HQ2x;
#endif
      MESSAGE_L ("Render selected : HQ2x", "Render selected : HQ2x", 1500)
	break;

    default:
      *Rend = DOUBLE;
#if (TARGET_SYS,x86)
      if (Have_MMX)
	*Blit = Blit_X2_MMX;
      else
	*Blit = Blit_X2;
#endif
      MESSAGE_L ("Render selected : DOUBLE", "Render selected : DOUBLE", 1500)
	break;
    }

  if (!((Full == Full_Screen) && ((Num >= 2) && (Old_Rend >= 2)) && (!Force)))
    {
      Shutdown_Video_SubSystem();

      if (Sound_Initialised)
	Clear_Sound_Buffer ();

      return (Init_Video_SubSystem
	      (320 * ((*Rend == 0) ? 1 : 2), 240 * ((*Rend == 0) ? 1 : 2),
	       SDL_SWSURFACE
//                                              |SDL_DOUBLEBUF
	       | ((Full_Screen = Full) ? SDL_FULLSCREEN : 0))
	      | SDL_ASYNCBLIT | SDL_HWACCEL);
    }
  else
    Flag_Clr_Scr = 1;
#else
  static int initted=0;
  if (!initted)
	  //Init_Video_SubSystem(320,240,SDL_FULLSCREEN|SDL_HWSURFACE);
	  Init_Video_SubSystem(640,480,SDL_FULLSCREEN|SDL_HWSURFACE);
  initted=1;
#endif
  return 0;
}

int Change_SegaCD_SRAM_Size (int num)
{
  if (num == -1)
    {
      BRAM_Ex_State &= 1;
    MESSAGE_L ("SegaCD SRAM cart removed", "SegaCD SRAM cart removed", 1500)}
  else
    {
      char bsize[256];

      BRAM_Ex_State |= 0x100;
      BRAM_Ex_Size = num;

      sprintf (bsize, "SegaCD SRAM cart plugged (%d Kb)", 8 << num);
    MESSAGE_L (bsize, bsize, 1500)}

  return 1;
}

int
Change_YM2612 (void)
{
  if (YM2612_Enable)
    {
      YM2612_Enable = 0;
    MESSAGE_L ("YM2612 Disabled", "YM2612 Disabled", 1000)}
  else
    {
      YM2612_Enable = 1;
    MESSAGE_L ("YM2612 Enabled", "YM2612 Enabled", 1000)}


  return (1);
}

int
Change_PSG_Improv (void)
{
  if (PSG_Improv)
    {
      PSG_Improv = 0;
    MESSAGE_L ("Normal PSG sound", "Normal PSG sound", 1000)}
  else
    {
      PSG_Improv = 1;
    MESSAGE_L ("Improved PSG sound", "Improved PSG sound", 1000)}

  return 1;
}

int
Change_YM2612_Improv (void)
{
  unsigned char Reg_1[0x200];

  if (YM2612_Improv)
    {
      YM2612_Improv = 0;
    MESSAGE_L ("Normal YM2612 emulation", "Normal YM2612 emulation", 1000)}
  else
    {
      YM2612_Improv = 1;
    MESSAGE_L ("High Quality YM2612 emulation",
		 "High Quality YM2612 emulation", 1000)}

  YM2612_Save (Reg_1);

  if (CPU_Mode)
    {
      YM2612_Init (CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
    }
  else
    {
      YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
    }

  YM2612_Restore (Reg_1);


  return 1;
}

#ifdef SEGA_CD
int Change_CDDA (void)
{
	if (CDDA_Enable)
	{
		CDDA_Enable = 0;
		MESSAGE_L ("CD Audio Disabled", "CD Audio Disabled", 1000)}
	else
	{
		CDDA_Enable = 1;
		MESSAGE_L ("CD Audio Enabled", "CD Audio Enabled", 1000)
	}
	
	
	return (1);
}
#endif

int Change_Fast_Blur (void)
{
  Flag_Clr_Scr = 1;

  if ((Fast_Blur = (1 - Fast_Blur)))
    MESSAGE_L ("Fast Blur Enabled", "Fast Blur Enabled", 1000)
    else
    MESSAGE_L ("Fast Blur Disabled", "Fast Blur Disabled", 1000) return (1);
}

int
Change_DAC_Improv (void)
{
  if (DAC_Improv)
    {
      DAC_Improv = 0;
    MESSAGE_L ("Normal DAC sound", "Normal DAC sound", 1000)}
  else
    {
      DAC_Improv = 1;
    MESSAGE_L ("Improved DAC sound (voices)", "Improved DAC sound (voices)",
		 1000)}

  return (1);
}

int
Change_DAC (void)
{
  if (DAC_Enable)
    {
      DAC_Enable = 0;
    MESSAGE_L ("DAC Disabled", "DAC Disabled", 1000)}
  else
    {
      DAC_Enable = 1;
    MESSAGE_L ("DAC Enabled", "DAC Enabled", 1000)}


  return (1);
}

int
Change_PSG (void)
{
  if (PSG_Enable)
    {
      PSG_Enable = 0;
    MESSAGE_L ("PSG Disabled", "PSG Disabled", 1000)}
  else
    {
      PSG_Enable = 1;
    MESSAGE_L ("PSG Enabled", "PSG Enabled", 1000)}


  return 1;
}

int
Change_PCM (void)
{
  if (PCM_Enable)
    {
      PCM_Enable = 0;
    MESSAGE_L ("PCM Sound Disabled", "PCM Sound Disabled", 1000)}
  else
    {
      PCM_Enable = 1;
    MESSAGE_L ("PCM Sound Enabled", "PCM Sound Enabled", 1000)}


  return 1;
}

#ifdef SEGA_32X
int Change_PWM (void)
{
  if (PWM_Enable)
    {
      PWM_Enable = 0;
    MESSAGE_L ("PWM Sound Disabled", "PWM Sound Disabled", 1000)}
  else
    {
      PWM_Enable = 1;
    MESSAGE_L ("PWM Sound Enabled", "PWM Sound Enabled", 1000)}


  return 1;
}
#endif

int
Change_Sound_Stereo (void)
{
  unsigned char Reg_1[0x200];

  if (Sound_Stereo)
    {
      Sound_Stereo = 0;
    MESSAGE_L ("Mono sound", "Mono sound", 1000)}
  else
    {
      Sound_Stereo = 1;
    MESSAGE_L ("Stereo sound", "Stereo sound", 1000)}

  if (Sound_Enable)
    {
      PSG_Save_State ();
      YM2612_Save (Reg_1);

      End_Sound ();
      Sound_Enable = 0;

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
#ifdef SEGA_CD
      if (SegaCD_Started)
	Set_Rate_PCM (Sound_Rate);
#endif
      YM2612_Restore (Reg_1);
      PSG_Restore_State ();

      if (!Init_Sound ())
	return (0);

      Sound_Enable = 1;
      Play_Sound ();
    }


  return (1);
}

int
Change_Sample_Rate (int Rate)
{
  unsigned char Reg_1[0x200];
  switch (Rate)
    {
    case 0:
      Sound_Rate = 11025;
      MESSAGE_L ("Sound rate set to 11025", "Sound rate set to 11025", 2500)
	break;

    case 1:
      Sound_Rate = 22050;
      MESSAGE_L ("Sound rate set to 22050", "Sound rate set to 22050", 2500)
	break;

    case 2:
      Sound_Rate = 44100;
      MESSAGE_L ("Sound rate set to 44100", "Sound rate set to 44100", 2500)
	break;
    default:
    	return 0;
    }

  if (Sound_Enable)
    {
      PSG_Save_State ();
      YM2612_Save (Reg_1);

      End_Sound ();
      Sound_Enable = 0;

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
#ifdef SEGA_CD
      if (SegaCD_Started)
	Set_Rate_PCM (Sound_Rate);
#endif
      YM2612_Restore (Reg_1);
      PSG_Restore_State ();

      if (!Init_Sound ())
	return (0);

      Sound_Enable = 1;
      Play_Sound ();
    }


  return (1);
}

int Change_Sound (void)
{
  if (Sound_Enable)
    {
      End_Sound ();

      Sound_Enable = 0;
      YM2612_Enable = 0;
      PSG_Enable = 0;
      DAC_Enable = 0;
#ifdef SEGA_CD
      PCM_Enable = 0;
#endif

#ifdef SEGA_32X
      PWM_Enable = 0;
#endif

#ifdef SEGA_CD
      CDDA_Enable = 0;
#endif

    MESSAGE_L ("Sound Disabled", "Sound Disabled", 1500)}
  else
    {
      if (!Init_Sound ())
	{
	  Sound_Enable = 0;
	  YM2612_Enable = 0;
	  PSG_Enable = 0;
	  DAC_Enable = 0;

#ifdef SEGA_32X
	  PWM_Enable = 0;
#endif

#ifdef SEGA_CD
	  PCM_Enable = 0;
	  CDDA_Enable = 0;
#endif

	  return 0;
	}

      Sound_Enable = 1;
      Play_Sound ();

	if (!(Z80_State & 1))
		Change_Z80 ();

      YM2612_Enable = 1;
      PSG_Enable = 1;
      DAC_Enable = 1;

#ifdef SEGA_32X
      PWM_Enable = 1;
#endif
    
#ifdef SEGA_CD
      PCM_Enable = 1;
      CDDA_Enable = 1;
#endif

    	MESSAGE_L ("Sound Enabled", "Sound Enabled", 1500)
	}

  return 1;
}

#ifdef SEGA_CD
int Change_SegaCD_Synchro (void)
{
  if (SegaCD_Accurate)
    {
      SegaCD_Accurate = 0;

      if (SegaCD_Started)
	{
	  Update_Frame = Do_SegaCD_Frame;
	  Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}

    MESSAGE_L ("SegaCD normal mode", "SegaCD normal mode", 1500)}
  else
    {
      SegaCD_Accurate = 1;

      if (SegaCD_Started)
	{
	  Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
	  Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}

    MESSAGE_L ("SegaCD perfect synchro mode (SLOW)",
		 "SegaCD perfect synchro mode (slower)", 1500)}


  return 1;
}
#endif

int Change_Country_Order (int Num)
{
  char c_str[4][4] = { "USA", "JAP", "EUR" };
  char str_w[128];
  int sav = Country_Order[Num];

  if (Num == 1)
    Country_Order[1] = Country_Order[0];
  else if (Num == 2)
    {
      Country_Order[2] = Country_Order[1];
      Country_Order[1] = Country_Order[0];
    }
  Country_Order[0] = sav;

  if (Country == -1)
    Change_Country (-1);	// Update Country

  sprintf (str_w, "Country detec.order : %s %s %s", c_str[Country_Order[0]],
	   c_str[Country_Order[1]], c_str[Country_Order[2]]);
  MESSAGE_L (str_w, str_w, 1500) return (1);
}

int Change_Z80 (void)
{
  if (Z80_State & 1)
    {
      Z80_State &= ~1;
    MESSAGE_L ("Z80 Disabled", "Z80 Disabled", 1000)}
  else
    {
      Z80_State |= 1;
    MESSAGE_L ("Z80 Enabled", "Z80 Enabled", 1000)}


  return (1);
}

int Change_Country (int Num)
{
	unsigned char Reg_1[0x200];

	Flag_Clr_Scr = 1;

	switch (Country = Num)
	{
		default:
			case -1:
				if (Genesis_Started
#ifdef SEGA_32X
					|| _32X_Started
#endif
				)
					Detect_Country_Genesis ();
#ifdef SEGA_CD
				else if (SegaCD_Started)
					Detect_Country_SegaCD ();
#endif
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

#ifdef SEGA_32X
      _32X_VDP.Mode &= ~0x8000;
#endif

#ifdef SEGA_CD
      CD_Access_Timer = 2080;
#endif

      //Timer_Step = 136752;
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

#ifdef SEGA_32X
      _32X_VDP.Mode |= 0x8000;
#endif

#ifdef SEGA_CD
      CD_Access_Timer = 2096;
#endif

      //Timer_Step = 135708;
    }

  if (Sound_Enable)
    {
      PSG_Save_State ();
      YM2612_Save (Reg_1);

      End_Sound ();
      Sound_Enable = 0;

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
#ifdef SEGA_CD

      if (SegaCD_Started)
	Set_Rate_PCM (Sound_Rate);
#endif
      YM2612_Restore (Reg_1);
      PSG_Restore_State ();

      if (!Init_Sound ())
	return (0);

      Sound_Enable = 1;
      Play_Sound ();
    }

  if (Game_Mode)
    {
      if (CPU_Mode)
	MESSAGE_L ("Europe system (50 FPS)", "Europe system (50 FPS)", 1500)
	else
	MESSAGE_L ("USA system (60 FPS)", "USA system (60 FPS)", 1500)}
	else
	{
	  if (CPU_Mode)
	    MESSAGE_L ("Japan system (50 FPS)", "Japan system (50 FPS)", 1500)
	    else
	    MESSAGE_L ("Japan system (60 FPS)", "Japan system (60 FPS)", 1500)}

	    if (Genesis_Started)
	      {
		if ((CPU_Mode == 1) || (Game_Mode == 0))
		  sprintf (Str_Tmp, "Gens - Megadrive : %s",
			   Game->Rom_Name_W);
		else
		  sprintf (Str_Tmp, "Gens - Genesis : %s", Game->Rom_Name_W);

		SetWindowText (Str_Tmp);
	      }
	    else if (_32X_Started)
	      {
		if (CPU_Mode == 1)
		  sprintf (Str_Tmp, "Gens - 32X (PAL) : %s",
			   Game->Rom_Name_W);
		else
		  sprintf (Str_Tmp, "Gens - 32X (NTSC) : %s",
			   Game->Rom_Name_W);

		SetWindowText (Str_Tmp);
	      }
	    else if (SegaCD_Started)
	      {
		if ((CPU_Mode == 1) || (Game_Mode == 0))
		  sprintf (Str_Tmp, "Gens - MegaCD : %s", Rom_Name);
		else
		  sprintf (Str_Tmp, "Gens - SegaCD : %s", Rom_Name);

		SetWindowText (Str_Tmp);
	      }


	  return 1;
	}
      int Change_Stretch (void)
      {
	if ((Full_Screen) && (Render_FS > 1))
	  return (0);

	Flag_Clr_Scr = 1;

	if ((Stretch = (1 - Stretch)))
	  MESSAGE_L ("Stretched mode", "Stretched mode", 1000)
	  else
	  MESSAGE_L ("Correct ratio mode", "Correct ratio mode", 1000)
	    return (1);
      }


      int Change_Blit_Style (void)
      {
	if ((!Full_Screen) || (Render_FS > 1))
	  return (0);

	Flag_Clr_Scr = 1;

	if ((Blit_Soft = (1 - Blit_Soft)))
	  MESSAGE_L ("Force software blit for Full-Screen",
		     "Force software blit for Full-Screen", 1000)
	  else
	  MESSAGE_L ("Enable hardware blit for Full-Screen",
		     "Enable hardware blit for Full-Screen", 1000) return (1);
      }

      int Set_Sprite_Over (int Num)
      {
	if ((Sprite_Over = Num))
	  MESSAGE_L ("Sprite Limit Enabled", "Sprite Limit Enabled", 1000)
	  else
	  MESSAGE_L ("Sprite Limit Disabled", "Sprite Limit Disabled", 1000)
	    return (1);
      }

      int Change_Debug (int Debug_Mode)
      {
	if (!Game)
	  return 0;

	Flag_Clr_Scr = 1;
	Clear_Sound_Buffer ();

	if (Debug_Mode == Debug)
	  Debug = 0;
	else
	  Debug = Debug_Mode;


	return 1;
      }
      int Change_VSync (void)
      {
	int *p_vsync;

	if (Full_Screen)
	  {
	    Shutdown_Video_SubSystem();
	    p_vsync = &FS_VSync;
	  }
	else
	  p_vsync = &W_VSync;

	*p_vsync = 1 - *p_vsync;

	if (*p_vsync)
	  MESSAGE_L ("Vertical Sync Enabled", "Vertical Sync Enabled", 1000)
	  else
	  MESSAGE_L ("Vertical Sync Disabled", "Vertical Sync Disabled", 1000)
	    if (Full_Screen)
	    return Init_Video_SubSystem(640, 480,
			       SDL_HWSURFACE | SDL_DOUBLEBUF |
			       SDL_FULLSCREEN);
	  else
	    return 1;
      }
      int Set_Frame_Skip (int Num)
      {
	Frame_Skip = Num;

	if (Frame_Skip != -1)
	  MESSAGE_NUM_L ("Frame skip set to %d", "Frame skip set to %d",
			 Frame_Skip, 1500)
	  else
	  MESSAGE_L ("Frame skip set to Auto", "Frame skip set to Auto", 1500)
	    return (1);
      }

      int Set_Current_State (int Num)
      {
	FILE *f;

	Current_State = Num;

	if ((f = Get_State_File ()))
	  {
	    fclose (f);
	  MESSAGE_NUM_L ("SLOT %d [OCCUPIED]", "SLOT %d [OCCUPIED]",
			   Current_State, 1500)}
	else
	  {
	  MESSAGE_NUM_L ("SLOT %d [EMPTY]", "SLOT %d [EMPTY]",
			   Current_State, 1500)}


	return 1;
      }

#if 0
void sync_gens_ui ()
{ 
}

void open_game_genie ()
{
}
#endif

