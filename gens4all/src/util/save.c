#include <assert.h>
#include <dc/vmu_pkg.h>
#include "gens4all.h"

#include "input.h"
#include "save.h"
#include "sh2.h"
#include "cd_aspi.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "scrshot.h"
#include "ggenie.h"
#include "io.h"
#include "cd_sys.h"
#include "gamelist.h"

#ifdef RAZE
#else
#include "stef/z80.h"
#endif

#ifndef SAVE_TO_PC
//This should be set in the Makefile, but if it's not, default to PC save
//Set to 1 to save to PC, set to 0 to save to VMU
#define SAVE_TO_PC	1
#endif

extern void watchdog_pause();
extern void watchdog_resume();

// CHUI: Changed key defaults for Dreamcast
enum
{
  DIK_UP = SDLK_UP, DIK_DOWN = SDLK_DOWN,
  DIK_LEFT = SDLK_LEFT, DIK_RIGHT = SDLK_RIGHT,
  DIK_RETURN = SDLK_RETURN, DIK_A = SDLK_LCTRL,
  DIK_S = SDLK_LALT, DIK_D = SDLK_SPACE,
  DIK_RSHIFT = SDLK_LSHIFT, DIK_Z = SDLK_TAB,
  DIK_X = SDLK_BACKSPACE, DIK_C = SDLK_0, DIK_Y = SDLK_w,
  DIK_H = SDLK_s, DIK_G = SDLK_a, DIK_J = SDLK_d,
  DIK_U = SDLK_z, DIK_K = SDLK_e, DIK_L = SDLK_q,
  DIK_M = SDLK_x, DIK_T = SDLK_c, DIK_I = SDLK_1,
  DIK_O = SDLK_2, DIK_P = SDLK_3
};

#define DIR_SEPARATOR '/'

int Current_State = 0;
char State_Dir[1024] = "";
char SRAM_Dir[1024] = "";
char BRAM_Dir[1024] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGHT];;


int
Change_File_S (char *Dest, char *Dir)
{
/*
  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Save state", GTK_FILE_CHOOSER_ACTION_SAVE);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename)
    {
      strcpy (Dest, filename);
      g_free (filename);
      return 1;
    }
*/
  return 0;
}


int
Change_File_L (char *Dest, char *Dir)
{
/*
  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Load state", GTK_FILE_CHOOSER_ACTION_OPEN);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename)
    {
      g_free (filename);
      strcpy (Dest, filename);
      return 1;
    }
*/
  return 0;
}

int
Change_File_L_CD (char *Dest, char *Dir)
{
/*
  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Load SegaCD image file",
				GTK_FILE_CHOOSER_ACTION_OPEN);
  addIsoFilter (widget);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename)
    {
      g_free (filename);
      strcpy (Dest, filename);
      return 1;
    }
*/
  return 0;
}

/*
int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	int i;
	gchar* filename=NULL;
	static FileSelData fd;
	GtkWidget* widget;
	GList* extensions=NULL;

	widget = init_file_selector(&fd, Titre);
	extensions = g_list_append(extensions,Ext);
	file_selector_add_type(&fd,"", extensions);
	gtk_widget_show(widget);
	filename = fileselection_get_filename(fd.filesel);	

	if (filename)
		{
		i = strlen(Dest) - 1;
		while ((i > 0) && (Dest[i] != DIR_SEPARATOR)) i--;
		if (!i) return 0;
		Dest[++i] = 0;
		g_free(filename);
		return 1;
	}
	return 0;
}

*/

FILE * Get_State_File ()
{
  char Name[2048];
  char Ext[5] = ".gsX";

  SetCurrentDirectory (Gens_Path);

  Ext[3] = '0' + Current_State;
  strcpy (Name, State_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, Ext);

  return fopen (Name, "rb");
}


void
Get_State_File_Name (char *name)
{
  char Ext[5] = ".gsX";

  SetCurrentDirectory (Gens_Path);

  Ext[3] = '0' + Current_State;
  strcpy (name, State_Dir);
  strcat (name, Rom_Name);
  strcat (name, Ext);
}



int
Load_State (char *Name)
{
	//~ FILE *f;
	int f;
	unsigned char *buf;
	int len;

	if (Genesis_Started)
		len = GENESIS_STATE_FILE_LENGHT_EX;
	else if (SegaCD_Started)
		len = SEGACD_STATE_FILE_LENGHT;
	else if (_32X_Started)
		len = G32X_STATE_FILE_LENGHT;
	else
		return 0;

	buf = State_Buffer;

	if ((f = open (Name, O_RDONLY)) == -1)
		return 0;

	memset (buf, 0, len);
	watchdog_pause();
	if (read (f, buf, len))
		{
#ifdef RAZE
	z80_reset();
#else
			z80_Reset (&M_Z80);
#endif
//              main68k_reset();
//              YM2612ResetChip(0);
//              Reset_VDP();

			Import_Genesis (buf);
			buf += 0x22500;
			if (SegaCD_Started)
			{
		//                      Import_SegaCD(buf);
				buf += 0xE0000;
			}

#ifdef SEGA_32X
			if (_32X_Started)
			{
				Import_32X (buf);
				buf += 0x82A00;
			}
#endif

			Flag_Clr_Scr = 1;
			CRam_Flag = 1;
			VRam_Flag = 1;

			sprintf (Str_Tmp, "STATE %d LOADED", Current_State);
			Put_Info (Str_Tmp, 2000);
		}

	close (f);
	watchdog_resume();

	return 1;
}


int
Save_State (char *Name)
{
	if (Name == NULL)
		return 0;
	FILE *f;
	unsigned char *buf;
	int len;

	buf = State_Buffer;
	if ((f = fopen (Name, "wb")) == NULL)
		return 0;

	//~ if (Genesis_Started)
		len = GENESIS_STATE_FILE_LENGHT;
	//~ else if (SegaCD_Started)
		//~ len = SEGACD_STATE_FILE_LENGHT;
	//~ else if (_32X_Started)
		//~ len = G32X_STATE_FILE_LENGHT;

	if (buf == NULL)
		return 0;
	memset (buf, 0, len);
	Export_Genesis (buf);
	buf += 0x22500;

#ifdef SEGA_CD
	if (SegaCD_Started)
		{
//              Export_SegaCD(buf);
			buf += 0xE0000;
		}
#endif

#ifdef SEGA_32X
	if (_32X_Started)
		{
			Export_32X (buf);
			buf += 0x82A00;
		}
#endif
	watchdog_pause();
	fwrite (State_Buffer, 1, len, f);
	fclose (f);
	watchdog_resume();

	sprintf (Str_Tmp, "STATE %d SAVED", Current_State);
	Put_Info (Str_Tmp, 2000);

	return 1;
}


/*

GST genecyst save file

Range        Size   Description
-----------  -----  -----------
00000-00002  3      "GST"
00006-00007  2      "\xE0\x40"
000FA-00112  24     VDP registers
00112-00191  128    Color RAM
00192-001E1  80     Vertical scroll RAM
001E4-003E3  512    YM2612 registers
00474-02473  8192   Z80 RAM
02478-12477  65536  68K RAM
12478-22477  65536  Video RAM

main 68000 registers
--------------------

00080-0009F : D0-D7
000A0-000BF : A0-A7
000C8 : PC
000D0 : SR
000D2 : USP
000D6 : SSP

Z80 registers
-------------

00404 : AF
00408 : BC
0040C : DE
00410 : HL
00414 : IX
00418 : IY
0041C : PC
00420 : SP
00424 : AF'
00428 : BC'
0042C : DE'
00430 : HL'

00434 : I
00435 : Unknow
00436 : IFF1 = IFF2
00437 : Unknow

The 'R' register is not supported.

Z80 State
---------

00438 : Z80 RESET
00439 : Z80 BUSREQ
0043A : Unknow
0043B : Unknow

0043C : Z80 BANK (DWORD)

Gens and Kega ADD
-----------------

00040 : last VDP Control data written (DWORD)
00044 : second write flag (1 for second write) 
00045 : DMA Fill flag (1 mean next data write will cause a DMA fill)
00048 : VDP write address (DWORD)

00050 : Version       (Genecyst=0 ; Kega=5 ; Gens=5)
00051 : Emulator ID   (Genecyst=0 ; Kega=0 ; Gens=1)
00052 : System ID     (Genesis=0 ; SegaCD=1 ; 32X=2 ; SegaCD32X=3)

00060-00070 : PSG registers (WORD).


SEGA CD
-------

+00000-00FFF : Gate array & sub 68K
+01000-80FFF : Prg RAM
+81000-C0FFF : Word RAM (2M mode arrangement)
+C1000-D0FFF : PCM RAM
+D1000-DFFFF : CDD & CDC data (16 kB cache include)

32X
---

main SH2
--------

+00000-00FFF : cache
+01000-011FF : IO registers
+01200-0123F : R0-R15
+01240 : SR
+01244 : GBR
+01248 : VBR
+0124C : MACL
+01250 : MACH
+01254 : PR
+01258 : PC
+0125C : State

sub SH2
-------

+01400-023FF : cache
+02400-025FF : IO registers
+02600-0263F : R0-R15
+02640 : SR
+02644 : GBR
+02648 : VBR
+0264C : MACL
+02650 : MACH
+02654 : PR
+02658 : PC
+0265C : State

others
------
// Fix 32X save state :
// enregistrer correctement les registres syst??mes ...

+02700 : ADEN bit (bit 0)
+02701 : FM bit (bit 7)
+02702 : Master SH2 INT mask register
+02703 : Slave SH2 INT mask register
+02704 : 68000 32X rom bank register
+02705 : RV (Rom to VRAM DMA allowed) bit (bit 0)
+02710-0273F : FIFO stuff (not yet done)
+02740-0274F : 32X communication buffer
+02750-02759 : PWM registers
+02760-0276F : 32X VDP registers
+02800-029FF : 32X palette
+02A00-429FF : SDRAM
+42A00-829FF : FB1 & FB2

*/

unsigned char IntToBCD(int val) {
	return ((val / 10) << 4) | (val % 10);
}
unsigned char CalcDOW(int d, int m, int y) {
	return (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
}

void DCTimeConvert(unsigned long int seconds, vmu_timestamp_t *dst)
{
	//https://www.geeksforgeeks.org/convert-unix-timestamp-to-dd-mm-yyyy-hhmmss-format/
	long int oseconds = seconds;
	// Save the time in Human
	// readable format

	// Number of days in month
	// in normal year
	int daysOfMonth[] = { 31, 28, 31, 30, 31, 30,
		31, 31, 30, 31, 30, 31 };

	long int currYear, daysTillNow, extraTime,
	extraDays, index, date, month, hours,
	minutes, secondss, flag = 0;

	// Calculate total days unix time T
	daysTillNow = seconds / (24 * 60 * 60);
	extraTime = seconds % (24 * 60 * 60);
	currYear = 1970;
 
	// Calculating current year
	while (daysTillNow >= 365) {
		if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0)) {
			daysTillNow -= 366;
		} else {
			daysTillNow -= 365;
		}
		currYear += 1;
	}
 
	// Updating extradays because it
	// will give days till previous day
	// and we have include current day
	extraDays = daysTillNow + 1;

	if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0))
		flag = 1;
 
	// Calculating MONTH and DATE
	month = 0, index = 0;
	if (flag == 1) {
		while (1) {
			if (index == 1) {
				if (extraDays - 29 < 0)
					break;
				month += 1;
				extraDays -= 29;
			} else {
				if (extraDays - daysOfMonth[index] < 0)
					break;
				month += 1;
				extraDays -= daysOfMonth[index];
			}
			index += 1;
		}
	} else {
		while (1) {
			if (extraDays - daysOfMonth[index] < 0)
				break;
			month += 1;
			extraDays -= daysOfMonth[index];
			index += 1;
		}
	}
 
	// Current Month
	if (extraDays > 0) {
		month += 1;
		date = extraDays;
	} else {
		if (month == 2 && flag == 1)
			date = 29;
		else {
			date = daysOfMonth[month - 1];
		}
	}
	
	// Calculating HH:MM:YYYY
	hours = extraTime / 3600;
	minutes = (extraTime % 3600) / 60;
	secondss = (extraTime % 3600) % 60;
	
	dst->cent = IntToBCD(currYear / 100);
	dst->year = IntToBCD(currYear % 100);
	dst->month = IntToBCD(month);
	dst->day = IntToBCD(date);
	dst->hour = IntToBCD(hours);
	dst->min = IntToBCD(minutes);
	dst->sec = IntToBCD(secondss);
	dst->dow = CalcDOW(date, month, currYear); //dow;
	
	
	//~ tPrintf("%i-%i-%i %i:%02i:%02i (%08x)", currYear, month, date, hours, minutes, secondss, oseconds);
}


void
Import_Genesis (unsigned char *Data)
{
//~ #if 0
  unsigned char Reg_1[0x200], Version, *src;
  int i;

  VDP_Int = 0;
  DMAT_Lenght = 0;
  Version = Data[0x50];
	assert(Version == 0xdc);

  for (i = 0; i < 0x80; i++)
    CRam[i] = Data[i + 0x112];
  for (i = 0; i < 0x50; i++)
    VSRam[i] = Data[i + 0x192];
  for (i = 0; i < 0x2000; i++)
    Ram_Z80[i] = Data[i + 0x474];

  for (i = 0; i < 0x10000; i += 2)
    {
      Ram_68k[i + 0] = Data[i + 0x2478 + 1];
      Ram_68k[i + 1] = Data[i + 0x2478 + 0];
    }

  for (i = 0; i < 0x10000; i += 2)
    {
      VRam[i + 0] = Data[i + 0x12478 + 1];
      VRam[i + 1] = Data[i + 0x12478 + 0];
    }

  for (i = 0; i < 0x200; i++)
    Reg_1[i] = Data[i + 0x1E4];
  YM2612_Restore (Reg_1);

  if ((Version >= 2) && (Version < 4))
    {
      src = (unsigned char *) &Ctrl;
      for (i = 0; i < 7 * 4; i++)
	*src++ = Data[i + 0x30];

      Z80_State &= ~6;
      if (Data[0x440] & 1)
	Z80_State |= 2;
      if (Data[0x444] & 1)
	Z80_State |= 4;

      src = (unsigned char *) &Bank_Z80;
      for (i = 0; i < 4; i++)
	*src++ = Data[i + 0x448];

      src = (unsigned char *) &PSG_Save;
      for (i = 0; i < 8 * 4; i++)
	*src++ = Data[i + 0x224B8];
      PSG_Restore_State ();
    }
  else if ((Version >= 4) || (Version == 0))	// New version compatible with Kega.
    {
      Z80_State &= ~6;

      if (Version == 4)
	{
	  //~ M_Z80.IM = Data[0x437];
	  //~ M_Z80.IFF.b.IFF1 = (Data[0x438] & 1) << 2;
	  //~ M_Z80.IFF.b.IFF2 = (Data[0x438] & 1) << 2;

	  Z80_State |= (Data[0x439] & 1) << 1;
	}
      else
	{
	  z80_set_reg(Z80_REG_IM, 1);
	  z80_set_reg(Z80_REG_IFF1, Data[0x436] & 1);
	  z80_set_reg(Z80_REG_IFF1, (Data[0x436] >> 1) & 1);

	  Z80_State |= ((Data[0x439] & 1) ^ 1) << 1;
	  Z80_State |= ((Data[0x438] & 1) ^ 1) << 2;
	}

      src = (unsigned char *) &Ctrl;
      for (i = 0; i < 7 * 4; i++)
	*src++ = 0;

      Write_VDP_Ctrl (Data[0x40] + (Data[0x41] << 8));
      Write_VDP_Ctrl (Data[0x42] + (Data[0x43] << 8));

      Ctrl.Flag = Data[0x44];
      Ctrl.DMA = (Data[0x45] & 1) << 2;
      Ctrl.Address = Data[0x48] + (Data[0x49] << 8);

      src = (unsigned char *) &Bank_Z80;
      for (i = 0; i < 4; i++)
	*src++ = Data[i + 0x43C];

      if (Version >= 4)
	{
	  for (i = 0; i < 8; i++)
	    PSG_Save[i] = Data[i * 2 + 0x60] + (Data[i * 2 + 0x61] << 8);
	  PSG_Restore_State ();
	}
    }

  z80_set_reg(Z80_REG_AF, Data[0x404] + (Data[0x405] << 8));
  z80_set_reg(Z80_REG_BC, Data[0x408] + (Data[0x409] << 8));
  z80_set_reg(Z80_REG_DE, Data[0x40C] + (Data[0x40D] << 8));
  z80_set_reg(Z80_REG_HL, Data[0x410] + (Data[0x411] << 8));
  z80_set_reg(Z80_REG_IX, Data[0x414] + (Data[0x415] << 8));
  z80_set_reg(Z80_REG_IY, Data[0x418] + (Data[0x419] << 8));
  z80_set_reg(Z80_REG_PC, Data[0x41C] + (Data[0x41D] << 8));
  z80_set_reg(Z80_REG_SP, Data[0x420] + (Data[0x421] << 8));
  z80_set_reg(Z80_REG_AF2, Data[0x424] + (Data[0x425] << 8));
  z80_set_reg(Z80_REG_BC2, Data[0x428] + (Data[0x429] << 8));
  z80_set_reg(Z80_REG_DE2, Data[0x42C] + (Data[0x42D] << 8));
  z80_set_reg(Z80_REG_HL2, Data[0x430] + (Data[0x431] << 8));
  z80_set_reg(Z80_REG_IR, Data[0x434] & 0xFF);

  M68K_CONTEXT c68k;
  m68k_get_context (&c68k);

  for (i = 0; i < 24; i++)
    Set_VDP_Reg (i, Data[0xFA + i]);

  src = (unsigned char *) &c68k.dreg[0];
  for (i = 0; i < 8 * 2 * 4; i++)
    *src++ = Data[0x80 + i];

  src = (unsigned char *) &c68k.pc;
  for (i = 0; i < 4; i++)
    *src++ = Data[0xC8 + i];

  src = (unsigned char *) &c68k.sr;
  for (i = 0; i < 2; i++)
    *src++ = Data[0xD0 + i];

  if ((Version >= 3) || (Version == 0))
    {
      if (Data[0xD1] & 0x20)
	{
	  // Supervisor
	  src = (unsigned char *) &c68k.asp;
	  for (i = 0; i < 2; i++)
	    *src++ = Data[0xD2 + i];
	}
      else
	{
	  // User
	  src = (unsigned char *) &c68k.asp;
	  for (i = 0; i < 2; i++)
	    *src++ = Data[0xD6 + i];
	}
    }

  m68k_set_context (&c68k);

//~ #endif
}


void Export_Genesis (unsigned char *Data)
{
//~ #if 0
  unsigned char Reg_1[0x200], *src;
  int i;

  while (DMAT_Lenght)
    Update_DMA ();		// Be sure to finish DMA before save

  Data[0x00] = 'G';
  Data[0x01] = 'S';
  Data[0x02] = 'T';
  Data[0x03] = 0x40;
  Data[0x04] = 0xE0;

  Data[0x50] = 0xdc;		// Version
  Data[0x51] = 0;		// Gens

  PSG_Save_State ();

  for (i = 0; i < 8; i++)
    {
      Data[0x60 + i * 2] = PSG_Save[i] & 0xFF;
      Data[0x61 + i * 2] = (PSG_Save[i] >> 8) & 0xFF;
    }

  //~ main68k_GetContext (&Context_68K);
  M68K_CONTEXT c68k;
  m68k_get_context(&c68k);

  src = (unsigned char *) &(c68k.dreg[0]);
  for (i = 0; i < 8 * 2 * 4; i++)
    Data[0x80 + i] = *src++;

  src = (unsigned char *) &(c68k.pc);
  for (i = 0; i < 4; i++)
    Data[0xC8 + i] = *src++;

  src = (unsigned char *) &(c68k.sr);
  for (i = 0; i < 2; i++)
    Data[0xD0 + i] = *src++;

  if (c68k.sr & 0x2000)
    {
      src = (unsigned char *) &(c68k.asp);
      for (i = 0; i < 4; i++)
	Data[0xD2 + i] = *src++;
      src = (unsigned char *) &(c68k.areg[7]);
      for (i = 0; i < 4; i++)
	Data[0xD6 + i] = *src++;
    }
  else
    {
      src = (unsigned char *) &(c68k.asp);
      for (i = 0; i < 4; i++)
	Data[0xD6 + i] = *src++;
      src = (unsigned char *) &(c68k.areg[7]);
      for (i = 0; i < 4; i++)
	Data[0xD2 + i] = *src++;
    }

  src = (unsigned char *) &(Ctrl.Data);
  for (i = 0; i < 4; i++)
    Data[0x40 + i] = *src++;

  Data[0x44] = Ctrl.Flag;
  Data[0x45] = (Ctrl.DMA >> 2) & 1;

  Data[0x48] = Ctrl.Address & 0xFF;
  Data[0x49] = (Ctrl.Address >> 8) & 0xFF;

  VDP_Reg.DMA_Lenght_L = VDP_Reg.DMA_Lenght & 0xFF;
  VDP_Reg.DMA_Lenght_H = (VDP_Reg.DMA_Lenght >> 8) & 0xFF;

  VDP_Reg.DMA_Src_Adr_L = VDP_Reg.DMA_Address & 0xFF;
  VDP_Reg.DMA_Src_Adr_M = (VDP_Reg.DMA_Address >> 8) & 0xFF;
  VDP_Reg.DMA_Src_Adr_H = (VDP_Reg.DMA_Address >> 16) & 0xFF;

  VDP_Reg.DMA_Src_Adr_H |= Ctrl.DMA_Mode & 0xC0;

  src = (unsigned char *) &(VDP_Reg.Set1);
  for (i = 0; i < 24; i++)
    {
      Data[0xFA + i] = *src;
      src += sizeof(VDPReg_t);
    }

  for (i = 0; i < 0x80; i++)
    Data[i + 0x112] = CRam[i];
  for (i = 0; i < 0x50; i++)
    Data[i + 0x192] = VSRam[i];

  YM2612_Save (Reg_1);
  for (i = 0; i < 0x200; i++)
    Data[i + 0x1E4] = Reg_1[i];

  //~ Data[0x404] = (unsigned char) (z80_Get_AF (&M_Z80) & 0xFF);
  Data[0x404] = (unsigned char) (z80_get_reg(Z80_REG_AF) & 0xFF);
  Data[0x405] = (unsigned char) (z80_get_reg(Z80_REG_AF) >> 8);
  Data[0x408] = (unsigned char) (z80_get_reg(Z80_REG_BC) & 0xFF);
  Data[0x409] = (unsigned char) (z80_get_reg(Z80_REG_BC) >> 8);
  Data[0x40C] = (unsigned char) (z80_get_reg(Z80_REG_DE) & 0xFF);
  Data[0x40D] = (unsigned char) (z80_get_reg(Z80_REG_DE) >> 8);
  Data[0x410] = (unsigned char) (z80_get_reg(Z80_REG_HL) & 0xFF);
  Data[0x411] = (unsigned char) (z80_get_reg(Z80_REG_HL) >> 8);
  Data[0x414] = (unsigned char) (z80_get_reg(Z80_REG_IX) & 0xFF);
  Data[0x415] = (unsigned char) (z80_get_reg(Z80_REG_IX) >> 8);
  Data[0x418] = (unsigned char) (z80_get_reg(Z80_REG_IY) & 0xFF);
  Data[0x419] = (unsigned char) (z80_get_reg(Z80_REG_IY) >> 8);
  Data[0x41C] = (unsigned char) (z80_get_reg(Z80_REG_PC) & 0xFF);
  Data[0x41D] = (unsigned char) (z80_get_reg(Z80_REG_PC) >> 8);
  Data[0x420] = (unsigned char) (z80_get_reg(Z80_REG_SP) & 0xFF);
  Data[0x421] = (unsigned char) (z80_get_reg(Z80_REG_SP) >> 8);
  Data[0x424] = (unsigned char) (z80_get_reg(Z80_REG_AF2) & 0xFF);
  Data[0x425] = (unsigned char) (z80_get_reg(Z80_REG_AF2) >> 8);
  Data[0x428] = (unsigned char) (z80_get_reg(Z80_REG_BC2) & 0xFF);
  Data[0x429] = (unsigned char) (z80_get_reg(Z80_REG_BC2) >> 8);
  Data[0x42C] = (unsigned char) (z80_get_reg(Z80_REG_DE2) & 0xFF);
  Data[0x42D] = (unsigned char) (z80_get_reg(Z80_REG_DE2) >> 8);
  Data[0x430] = (unsigned char) (z80_get_reg(Z80_REG_HL2) & 0xFF);
  Data[0x431] = (unsigned char) (z80_get_reg(Z80_REG_HL2) >> 8);
  Data[0x434] = (unsigned char) (z80_get_reg(Z80_REG_IR));
  Data[0x436] = (unsigned char) (z80_get_reg(Z80_REG_IFF1) | (z80_get_reg(Z80_REG_IFF2)<<1));

  Data[0x438] = (unsigned char) (((Z80_State & 4) >> 2) ^ 1);
  Data[0x439] = (unsigned char) (((Z80_State & 2) >> 1) ^ 1);

  src = (unsigned char *) &Bank_Z80;
  for (i = 0; i < 4; i++)
    Data[0x43C + i] = *src++;

  for (i = 0; i < 0x2000; i++)
    Data[i + 0x474] = Ram_Z80[i];

  for (i = 0; i < 0x10000; i += 2)
    {
      Data[i + 0x2478 + 1] = Ram_68k[i + 0];
      Data[i + 0x2478 + 0] = Ram_68k[i + 1];
    }

  for (i = 0; i < 0x10000; i += 2)
    {
      Data[i + 0x12478 + 1] = VRam[i + 0];
      Data[i + 0x12478 + 0] = VRam[i + 1];
    }
//~ #endif
}

#ifdef SEGA_32X
void Import_32X (unsigned char *Data)
{
  unsigned char *src;
  int i;

  for (i = 0; i < 0x1000; i++)
    M_SH2.Cache[i] = Data[i];
  for (i = 0; i < 0x200; i++)
    SH2_Write_Byte (&M_SH2, 0xFFFFFE00 + i, Data[0x1000 + i]);

  src = (unsigned char *) &(M_SH2.R[0]);
  for (i = 0; i < 16 * 4; i++)
    *src++ = Data[0x1200 + i];

  SH2_Set_SR (&M_SH2,
	      (Data[0x1243] << 24) + (Data[0x1242] << 16) +
	      (Data[0x1241] << 8) + Data[0x1240]);
  SH2_Set_GBR (&M_SH2,
	       (Data[0x1247] << 24) + (Data[0x1246] << 16) +
	       (Data[0x1245] << 8) + Data[0x1244]);
  SH2_Set_VBR (&M_SH2,
	       (Data[0x124B] << 24) + (Data[0x124A] << 16) +
	       (Data[0x1249] << 8) + Data[0x1248]);
  SH2_Set_MACL (&M_SH2,
		(Data[0x124F] << 24) + (Data[0x124E] << 16) +
		(Data[0x124D] << 8) + Data[0x124C]);
  SH2_Set_MACH (&M_SH2,
		(Data[0x1253] << 24) + (Data[0x1252] << 16) +
		(Data[0x1251] << 8) + Data[0x1250]);
  SH2_Set_PR (&M_SH2,
	      (Data[0x1257] << 24) + (Data[0x1256] << 16) +
	      (Data[0x1255] << 8) + Data[0x1254]);
  SH2_Set_PC (&M_SH2,
	      (Data[0x125B] << 24) + (Data[0x125A] << 16) +
	      (Data[0x1259] << 8) + Data[0x1258]);
  M_SH2.Status =
    (Data[0x125F] << 24) + (Data[0x125E] << 16) + (Data[0x125D] << 8) +
    Data[0x125C];

  for (i = 0; i < 0x1000; i++)
    S_SH2.Cache[i] = Data[i + 0x1400];
  for (i = 0; i < 0x200; i++)
    SH2_Write_Byte (&S_SH2, 0xFFFFFE00 + i, Data[0x2400 + i]);

  src = (unsigned char *) &(S_SH2.R[0]);
  for (i = 0; i < 16 * 4; i++)
    *src++ = Data[0x2600 + i];

  SH2_Set_SR (&S_SH2,
	      (Data[0x2643] << 24) + (Data[0x2642] << 16) +
	      (Data[0x2641] << 8) + Data[0x2640]);
  SH2_Set_GBR (&S_SH2,
	       (Data[0x2647] << 24) + (Data[0x2646] << 16) +
	       (Data[0x2645] << 8) + Data[0x2644]);
  SH2_Set_VBR (&S_SH2,
	       (Data[0x264B] << 24) + (Data[0x264A] << 16) +
	       (Data[0x2649] << 8) + Data[0x2648]);
  SH2_Set_MACL (&S_SH2,
		(Data[0x264F] << 24) + (Data[0x264E] << 16) +
		(Data[0x264D] << 8) + Data[0x264C]);
  SH2_Set_MACH (&S_SH2,
		(Data[0x2653] << 24) + (Data[0x2652] << 16) +
		(Data[0x2651] << 8) + Data[0x2650]);
  SH2_Set_PR (&S_SH2,
	      (Data[0x2657] << 24) + (Data[0x2656] << 16) +
	      (Data[0x2655] << 8) + Data[0x2654]);
  SH2_Set_PC (&S_SH2,
	      (Data[0x265B] << 24) + (Data[0x265A] << 16) +
	      (Data[0x2659] << 8) + Data[0x2658]);
  S_SH2.Status =
    (Data[0x265F] << 24) + (Data[0x265E] << 16) + (Data[0x265D] << 8) +
    Data[0x265C];

  _32X_ADEN = Data[0x2700] & 1;
  _32X_RV = Data[0x2705] & 1;
  M68K_32X_Mode ();
  _32X_FM = Data[0x2701] & 0x80;
  _32X_Set_FB ();
  _32X_MINT = Data[0x2702];
  _32X_SINT = Data[0x2703];
  Bank_SH2 = Data[0x2704];
  M68K_Set_32X_Rom_Bank ();

	/*******
	FIFO stuff to add here...	
	 *******/
  for (i = 0; i < 0x10; i++)
    SH2_Write_Byte (&M_SH2, 0x4020 + i, Data[0x2740 + i]);
  for (i = 0; i < 4; i++)
    SH2_Write_Byte (&M_SH2, 0x4030 + i, Data[0x2750 + i]);
	/*******
	Extra PWM stuff to add here...	
	 *******/

  // Do it to allow VDP write on 32X side
  _32X_FM = 0x80;
  _32X_Set_FB ();

  for (i = 0; i < 0x10; i++)
    SH2_Write_Byte (&M_SH2, 0x4100 + i, Data[0x2760 + i]);
  for (i = 0; i < 0x200; i += 2)
    SH2_Write_Word (&M_SH2, 0x4200 + i,
		    Data[0x2800 + i + 0] + (Data[0x2800 + i + 1] << 8));
  for (i = 0; i < 0x40000; i++)
    SH2_Write_Byte (&M_SH2, 0x6000000 + i, Data[0x2A00 + i]);

  _32X_FM = Data[0x2701] & 0x80;
  _32X_Set_FB ();

  if (SH2_Read_Word (&M_SH2, 0x410A) & 1)
    {
      for (i = 0; i < 0x20000; i++)
	_32X_VDP_Ram[i] = Data[0x42A00 + i];
      for (i = 0; i < 0x20000; i++)
	_32X_VDP_Ram[i + 0x20000] = Data[0x62A00 + i];
    }
  else
    {
      for (i = 0; i < 0x20000; i++)
	_32X_VDP_Ram[i + 0x20000] = Data[0x42A00 + i];
      for (i = 0; i < 0x20000; i++)
	_32X_VDP_Ram[i] = Data[0x62A00 + i];
    }
}


void Export_32X (unsigned char *Data)
{
  unsigned char *src;
  int i;

  for (i = 0; i < 0x1000; i++)
    Data[i] = M_SH2.Cache[i];
  for (i = 0; i < 0x200; i++)
    Data[0x1000 + i] = M_SH2.IO_Reg[i];

  src = (unsigned char *) &(M_SH2.R[0]);
  for (i = 0; i < 16 * 4; i++)
    Data[0x1200 + i] = *src++;

  i = SH2_Get_SR (&M_SH2);
  Data[0x1240] = (unsigned char) (i & 0xFF);
  Data[0x1241] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x1242] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x1243] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_GBR (&M_SH2);
  Data[0x1244] = (unsigned char) (i & 0xFF);
  Data[0x1245] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x1246] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x1247] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_VBR (&M_SH2);
  Data[0x1248] = (unsigned char) (i & 0xFF);
  Data[0x1249] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x124A] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x124B] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_MACL (&M_SH2);
  Data[0x124C] = (unsigned char) (i & 0xFF);
  Data[0x124D] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x124E] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x124F] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_MACH (&M_SH2);
  Data[0x1250] = (unsigned char) (i & 0xFF);
  Data[0x1251] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x1252] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x1253] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_PR (&M_SH2);
  Data[0x1254] = (unsigned char) (i & 0xFF);
  Data[0x1255] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x1256] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x1257] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_PC (&M_SH2);
  Data[0x1258] = (unsigned char) (i & 0xFF);
  Data[0x1259] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x125A] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x125B] = (unsigned char) ((i >> 24) & 0xFF);
  i = M_SH2.Status;
  Data[0x125C] = (unsigned char) (i & 0xFF);
  Data[0x125D] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x125E] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x125F] = (unsigned char) ((i >> 24) & 0xFF);

  for (i = 0; i < 0x1000; i++)
    Data[0x1400 + i] = S_SH2.Cache[i];
  for (i = 0; i < 0x200; i++)
    Data[0x2400 + i] = S_SH2.IO_Reg[i];

  src = (unsigned char *) &(S_SH2.R[0]);
  for (i = 0; i < 16 * 4; i++)
    Data[0x2600 + i] = *src++;

  i = SH2_Get_SR (&S_SH2);
  Data[0x2640] = (unsigned char) (i & 0xFF);
  Data[0x2641] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x2642] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x2643] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_GBR (&S_SH2);
  Data[0x2644] = (unsigned char) (i & 0xFF);
  Data[0x2645] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x2646] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x2647] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_VBR (&S_SH2);
  Data[0x2648] = (unsigned char) (i & 0xFF);
  Data[0x2649] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x264A] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x264B] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_MACL (&S_SH2);
  Data[0x264C] = (unsigned char) (i & 0xFF);
  Data[0x264D] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x264E] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x264F] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_MACH (&S_SH2);
  Data[0x2650] = (unsigned char) (i & 0xFF);
  Data[0x2651] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x2652] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x2653] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_PR (&S_SH2);
  Data[0x2654] = (unsigned char) (i & 0xFF);
  Data[0x2655] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x2656] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x2657] = (unsigned char) ((i >> 24) & 0xFF);
  i = SH2_Get_PC (&S_SH2);
  Data[0x2658] = (unsigned char) (i & 0xFF);
  Data[0x2659] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x265A] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x265B] = (unsigned char) ((i >> 24) & 0xFF);
  i = S_SH2.Status;
  Data[0x265C] = (unsigned char) (i & 0xFF);
  Data[0x265D] = (unsigned char) ((i >> 8) & 0xFF);
  Data[0x265E] = (unsigned char) ((i >> 16) & 0xFF);
  Data[0x265F] = (unsigned char) ((i >> 24) & 0xFF);

  // Do it to allow VDP write on 32X side
  Data[0x2700] = _32X_ADEN & 1;
  Data[0x2705] = _32X_RV & 1;
  Data[0x2701] = _32X_FM & 0x80;
  Data[0x2702] = _32X_MINT;
  Data[0x2703] = _32X_SINT;
  Data[0x2704] = Bank_SH2;

	/*******
	FIFO stuff to add here...	
	 *******/
  for (i = 0; i < 0x10; i++)
    Data[0x2740 + i] = SH2_Read_Byte (&M_SH2, 0x4020 + i);
  for (i = 0; i < 4; i++)
    Data[0x2750 + i] = SH2_Read_Byte (&M_SH2, 0x4030 + i);
	/*******
	Extra PWM stuff to add here...	
	 *******/

  // Do it to allow VDP write on 32X side
  _32X_FM = 0x80;
  _32X_Set_FB ();

  for (i = 0; i < 0x10; i++)
    Data[0x2760 + i] = SH2_Read_Byte (&M_SH2, 0x4100 + i);
  for (i = 0; i < 0x200; i += 2)
    {
      Data[0x2800 + i + 0] =
	(unsigned char) (SH2_Read_Word (&M_SH2, 0x4200 + i) & 0xFF);
      Data[0x2800 + i + 1] =
	(unsigned char) (SH2_Read_Word (&M_SH2, 0x4200 + i) >> 8);
    }
  for (i = 0; i < 0x40000; i++)
    Data[0x2A00 + i] = SH2_Read_Byte (&M_SH2, 0x6000000 + i);

  _32X_FM = Data[0x2701] & 0x80;
  _32X_Set_FB ();

  if (SH2_Read_Word (&M_SH2, 0x410A) & 1)
    {
      for (i = 0; i < 0x20000; i++)
	Data[0x42A00 + i] = _32X_VDP_Ram[i];
      for (i = 0; i < 0x20000; i++)
	Data[0x62A00 + i] = _32X_VDP_Ram[i + 0x20000];
    }
  else
    {
      for (i = 0; i < 0x20000; i++)
	Data[0x42A00 + i] = _32X_VDP_Ram[i + 0x20000];
      for (i = 0; i < 0x20000; i++)
	Data[0x62A00 + i] = _32X_VDP_Ram[i];
    }
}
#endif

int
Save_Config (char *File_Name)
{
  char Conf_File[1024];

  strcpy (Conf_File, File_Name);

  WritePrivateProfileString ("General", "Rom path", Rom_Dir, Conf_File);
  WritePrivateProfileString ("General", "Save path", State_Dir, Conf_File);
  WritePrivateProfileString ("General", "SRAM path", SRAM_Dir, Conf_File);
  WritePrivateProfileString ("General", "BRAM path", BRAM_Dir, Conf_File);
  WritePrivateProfileString ("General", "Dump path", Dump_Dir, Conf_File);
  WritePrivateProfileString ("General", "Dump GYM path", Dump_GYM_Dir,
			     Conf_File);
  WritePrivateProfileString ("General", "Screen Shot path", ScrShot_Dir,
			     Conf_File);
  WritePrivateProfileString ("General", "Patch path", Patch_Dir, Conf_File);
  WritePrivateProfileString ("General", "IPS Patch path", IPS_Dir, Conf_File);

  WritePrivateProfileString ("General", "Genesis Bios", Genesis_Bios,
			     Conf_File);

  //~ WritePrivateProfileString ("General", "USA CD Bios", US_CD_Bios, Conf_File);
  //~ WritePrivateProfileString ("General", "EUROPE CD Bios", EU_CD_Bios,
			     //~ Conf_File);
  //~ WritePrivateProfileString ("General", "JAPAN CD Bios", JA_CD_Bios,
			     //~ Conf_File);

  //~ WritePrivateProfileString ("General", "32X 68000 Bios", _32X_Genesis_Bios,
			     //~ Conf_File);
  //~ WritePrivateProfileString ("General", "32X Master SH2 Bios",
			     //~ _32X_Master_Bios, Conf_File);
  //~ WritePrivateProfileString ("General", "32X Slave SH2 Bios", _32X_Slave_Bios,
			     //~ Conf_File);

  WritePrivateProfileString ("General", "Rom 1", Recent_Rom[0], Conf_File);
  WritePrivateProfileString ("General", "Rom 2", Recent_Rom[1], Conf_File);
  WritePrivateProfileString ("General", "Rom 3", Recent_Rom[2], Conf_File);
  WritePrivateProfileString ("General", "Rom 4", Recent_Rom[3], Conf_File);
  WritePrivateProfileString ("General", "Rom 5", Recent_Rom[4], Conf_File);
  WritePrivateProfileString ("General", "Rom 6", Recent_Rom[5], Conf_File);
  WritePrivateProfileString ("General", "Rom 7", Recent_Rom[6], Conf_File);
  WritePrivateProfileString ("General", "Rom 8", Recent_Rom[7], Conf_File);
  WritePrivateProfileString ("General", "Rom 9", Recent_Rom[8], Conf_File);

  sprintf (Str_Tmp, "%d", Current_State);
  WritePrivateProfileString ("General", "State Number", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Language);
  WritePrivateProfileString ("General", "Language", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Window_Pos.x);
  WritePrivateProfileString ("General", "Window X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Window_Pos.y);
  WritePrivateProfileString ("General", "Window Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Effect_Color);
  WritePrivateProfileString ("General", "Free Mode Color", Str_Tmp,
			     Conf_File);

  sprintf (Str_Tmp, "%d", Full_Screen & 1);
  WritePrivateProfileString ("Graphics", "Full Screen", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", FS_VSync & 1);
  WritePrivateProfileString ("Graphics", "Full Screen VSync", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", W_VSync & 1);
  WritePrivateProfileString ("Graphics", "Windows VSync", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Render_W);
  WritePrivateProfileString ("Graphics", "Render Windowed", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", Render_FS);
  WritePrivateProfileString ("Graphics", "Render Fullscreen", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", Stretch & 1);
  WritePrivateProfileString ("Graphics", "Stretch", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Blit_Soft & 1);
  WritePrivateProfileString ("Graphics", "Software Blit", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Contrast_Level);
  WritePrivateProfileString ("Graphics", "Contrast", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Brightness_Level);
  WritePrivateProfileString ("Graphics", "Brightness", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Greyscale & 1);
  WritePrivateProfileString ("Graphics", "Greyscale", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Invert_Color & 1);
  WritePrivateProfileString ("Graphics", "Invert", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Sprite_Over & 1);
  WritePrivateProfileString ("Graphics", "Sprite limit", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Frame_Skip);
  WritePrivateProfileString ("Graphics", "Frame skip", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Sound_Enable & 1);
  WritePrivateProfileString ("Sound", "State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Sound_Rate);
  WritePrivateProfileString ("Sound", "Rate", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Sound_Stereo);
  WritePrivateProfileString ("Sound", "Stereo", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Z80_State & 1);
  WritePrivateProfileString ("Sound", "Z80 State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", YM2612_Enable & 1);
  WritePrivateProfileString ("Sound", "YM2612 State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PSG_Enable & 1);
  WritePrivateProfileString ("Sound", "PSG State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", DAC_Enable & 1);
  WritePrivateProfileString ("Sound", "DAC State", Str_Tmp, Conf_File);
#ifdef SEGA_CD
  sprintf (Str_Tmp, "%d", PCM_Enable & 1);
#else
  sprintf (Str_Tmp, "%d", 0);
#endif
  WritePrivateProfileString ("Sound", "PCM State", Str_Tmp, Conf_File);

#ifdef SEGA_32X
  sprintf (Str_Tmp, "%d", PWM_Enable & 1);
  WritePrivateProfileString ("Sound", "PWM State", Str_Tmp, Conf_File);
#endif

#ifdef SEGA_CD
  sprintf (Str_Tmp, "%d", CDDA_Enable & 1);
  WritePrivateProfileString ("Sound", "CDDA State", Str_Tmp, Conf_File);
#endif

  sprintf (Str_Tmp, "%d", YM2612_Improv & 1);
  WritePrivateProfileString ("Sound", "YM2612 Improvement", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", DAC_Improv & 1);
  WritePrivateProfileString ("Sound", "DAC Improvement", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PSG_Improv & 1);
  WritePrivateProfileString ("Sound", "PSG Improvement", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Country);
  WritePrivateProfileString ("CPU", "Country", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[0]);
  WritePrivateProfileString ("CPU", "Prefered Country 1", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[1]);
  WritePrivateProfileString ("CPU", "Prefered Country 2", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[2]);
  WritePrivateProfileString ("CPU", "Prefered Country 3", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", SegaCD_Accurate);
  WritePrivateProfileString ("CPU",
			     "Perfect synchro between main and sub CPU (Sega CD)",
			     Str_Tmp, Conf_File);

#ifdef SEGA_32X
  sprintf (Str_Tmp, "%d", MSH2_Speed);
  WritePrivateProfileString ("CPU", "Main SH2 Speed", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", SSH2_Speed);
  WritePrivateProfileString ("CPU", "Slave SH2 Speed", Str_Tmp, Conf_File);
#endif

  sprintf (Str_Tmp, "%d", Fast_Blur & 1);
  WritePrivateProfileString ("Options", "Fast Blur", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_FPS & 1);
  WritePrivateProfileString ("Options", "FPS", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", FPS_Style);
  WritePrivateProfileString ("Options", "FPS Style", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_Message & 1);
  WritePrivateProfileString ("Options", "Message", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Message_Style);
  WritePrivateProfileString ("Options", "Message Style", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_LED & 1);
  WritePrivateProfileString ("Options", "LED", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Auto_Fix_CS & 1);
  WritePrivateProfileString ("Options", "Auto Fix Checksum", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", Auto_Pause & 1);
  WritePrivateProfileString ("Options", "Auto Pause", Str_Tmp, Conf_File);

#ifdef SEGA_CD
  sprintf (Str_Tmp, "%d", CUR_DEV);
  WritePrivateProfileString ("Options", "CD Drive", Str_Tmp, Conf_File);
#endif

  if (BRAM_Ex_State & 0x100)
    {
      sprintf (Str_Tmp, "%d", BRAM_Ex_Size);
      WritePrivateProfileString ("Options", "Ram Cart Size", Str_Tmp,
				 Conf_File);
    }
  else
    {
      WritePrivateProfileString ("Options", "Ram Cart Size", "-1", Conf_File);
    }

  WritePrivateProfileString ("Options", "GCOffline path", CGOffline_Path,
			     Conf_File);
  WritePrivateProfileString ("Options", "Gens manual path", Manual_Path,
			     Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1_Type & 0x13);
  WritePrivateProfileString ("Input", "P1.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Up);
  WritePrivateProfileString ("Input", "P1.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Down);
  WritePrivateProfileString ("Input", "P1.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Left);
  WritePrivateProfileString ("Input", "P1.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Right);
  WritePrivateProfileString ("Input", "P1.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Start);
  WritePrivateProfileString ("Input", "P1.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].A);
  WritePrivateProfileString ("Input", "P1.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].B);
  WritePrivateProfileString ("Input", "P1.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].C);
  WritePrivateProfileString ("Input", "P1.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Mode);
  WritePrivateProfileString ("Input", "P1.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].X);
  WritePrivateProfileString ("Input", "P1.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Y);
  WritePrivateProfileString ("Input", "P1.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Z);
  WritePrivateProfileString ("Input", "P1.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1B_Type & 0x03);
  WritePrivateProfileString ("Input", "P1B.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Up);
  WritePrivateProfileString ("Input", "P1B.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Down);
  WritePrivateProfileString ("Input", "P1B.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Left);
  WritePrivateProfileString ("Input", "P1B.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Right);
  WritePrivateProfileString ("Input", "P1B.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Start);
  WritePrivateProfileString ("Input", "P1B.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].A);
  WritePrivateProfileString ("Input", "P1B.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].B);
  WritePrivateProfileString ("Input", "P1B.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].C);
  WritePrivateProfileString ("Input", "P1B.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Mode);
  WritePrivateProfileString ("Input", "P1B.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].X);
  WritePrivateProfileString ("Input", "P1B.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Y);
  WritePrivateProfileString ("Input", "P1B.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Z);
  WritePrivateProfileString ("Input", "P1B.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1C_Type & 0x03);
  WritePrivateProfileString ("Input", "P1C.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Up);
  WritePrivateProfileString ("Input", "P1C.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Down);
  WritePrivateProfileString ("Input", "P1C.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Left);
  WritePrivateProfileString ("Input", "P1C.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Right);
  WritePrivateProfileString ("Input", "P1C.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Start);
  WritePrivateProfileString ("Input", "P1C.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].A);
  WritePrivateProfileString ("Input", "P1C.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].B);
  WritePrivateProfileString ("Input", "P1C.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].C);
  WritePrivateProfileString ("Input", "P1C.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Mode);
  WritePrivateProfileString ("Input", "P1C.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].X);
  WritePrivateProfileString ("Input", "P1C.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Y);
  WritePrivateProfileString ("Input", "P1C.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Z);
  WritePrivateProfileString ("Input", "P1C.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1D_Type & 0x03);
  WritePrivateProfileString ("Input", "P1D.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Up);
  WritePrivateProfileString ("Input", "P1D.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Down);
  WritePrivateProfileString ("Input", "P1D.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Left);
  WritePrivateProfileString ("Input", "P1D.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Right);
  WritePrivateProfileString ("Input", "P1D.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Start);
  WritePrivateProfileString ("Input", "P1D.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].A);
  WritePrivateProfileString ("Input", "P1D.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].B);
  WritePrivateProfileString ("Input", "P1D.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].C);
  WritePrivateProfileString ("Input", "P1D.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Mode);
  WritePrivateProfileString ("Input", "P1D.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].X);
  WritePrivateProfileString ("Input", "P1D.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Y);
  WritePrivateProfileString ("Input", "P1D.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Z);
  WritePrivateProfileString ("Input", "P1D.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2_Type & 0x13);
  WritePrivateProfileString ("Input", "P2.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Up);
  WritePrivateProfileString ("Input", "P2.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Down);
  WritePrivateProfileString ("Input", "P2.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Left);
  WritePrivateProfileString ("Input", "P2.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Right);
  WritePrivateProfileString ("Input", "P2.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Start);
  WritePrivateProfileString ("Input", "P2.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].A);
  WritePrivateProfileString ("Input", "P2.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].B);
  WritePrivateProfileString ("Input", "P2.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].C);
  WritePrivateProfileString ("Input", "P2.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Mode);
  WritePrivateProfileString ("Input", "P2.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].X);
  WritePrivateProfileString ("Input", "P2.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Y);
  WritePrivateProfileString ("Input", "P2.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Z);
  WritePrivateProfileString ("Input", "P2.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2B_Type & 0x03);
  WritePrivateProfileString ("Input", "P2B.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Up);
  WritePrivateProfileString ("Input", "P2B.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Down);
  WritePrivateProfileString ("Input", "P2B.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Left);
  WritePrivateProfileString ("Input", "P2B.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Right);
  WritePrivateProfileString ("Input", "P2B.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Start);
  WritePrivateProfileString ("Input", "P2B.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].A);
  WritePrivateProfileString ("Input", "P2B.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].B);
  WritePrivateProfileString ("Input", "P2B.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].C);
  WritePrivateProfileString ("Input", "P2B.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Mode);
  WritePrivateProfileString ("Input", "P2B.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].X);
  WritePrivateProfileString ("Input", "P2B.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Y);
  WritePrivateProfileString ("Input", "P2B.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Z);
  WritePrivateProfileString ("Input", "P2B.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2C_Type & 0x03);
  WritePrivateProfileString ("Input", "P2C.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Up);
  WritePrivateProfileString ("Input", "P2C.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Down);
  WritePrivateProfileString ("Input", "P2C.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Left);
  WritePrivateProfileString ("Input", "P2C.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Right);
  WritePrivateProfileString ("Input", "P2C.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Start);
  WritePrivateProfileString ("Input", "P2C.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].A);
  WritePrivateProfileString ("Input", "P2C.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].B);
  WritePrivateProfileString ("Input", "P2C.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].C);
  WritePrivateProfileString ("Input", "P2C.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Mode);
  WritePrivateProfileString ("Input", "P2C.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].X);
  WritePrivateProfileString ("Input", "P2C.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Y);
  WritePrivateProfileString ("Input", "P2C.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Z);
  WritePrivateProfileString ("Input", "P2C.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2D_Type & 0x03);
  WritePrivateProfileString ("Input", "P2D.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Up);
  WritePrivateProfileString ("Input", "P2D.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Down);
  WritePrivateProfileString ("Input", "P2D.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Left);
  WritePrivateProfileString ("Input", "P2D.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Right);
  WritePrivateProfileString ("Input", "P2D.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Start);
  WritePrivateProfileString ("Input", "P2D.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].A);
  WritePrivateProfileString ("Input", "P2D.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].B);
  WritePrivateProfileString ("Input", "P2D.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].C);
  WritePrivateProfileString ("Input", "P2D.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Mode);
  WritePrivateProfileString ("Input", "P2D.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].X);
  WritePrivateProfileString ("Input", "P2D.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Y);
  WritePrivateProfileString ("Input", "P2D.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Z);
  WritePrivateProfileString ("Input", "P2D.Z", Str_Tmp, Conf_File);

  return 1;
}



int
Save_As_Config ()
{
/*
  char Name[2048];
  char *filename;
  GtkWidget *widget;
  gint res;

  widget =
    create_file_chooser_dialog ("Save Config As",
				GTK_FILE_CHOOSER_ACTION_SAVE);
  addCfgFilter (widget);

  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  strcpy (Name, filename);
  g_free (filename);

  if (strlen (Name))
    {
      Save_Config (Name);
      strcpy (Str_Tmp, "config saved in ");
      strcat (Str_Tmp, Name);
      Put_Info (Str_Tmp, 2000);
      return 1;
    }
*/
  return 0;
}


int
Load_Config (char *File_Name, void *Game_Active)
{
  int new_val;
  char Conf_File[1024];

  dbgf("Load_Config '%s'\n",File_Name);
  SetCurrentDirectory (Gens_Path);
  strncpy (Conf_File, File_Name,1024);
  Conf_File[1023]=0;

  CRam_Flag = 1;

  GetPrivateProfileString ("General", "Rom path", default_path, &Rom_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Save path", Rom_Dir, &State_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "SRAM path", Rom_Dir, &SRAM_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "BRAM path", Rom_Dir, &BRAM_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Dump path", Rom_Dir, &Dump_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Dump GYM path", Rom_Dir,
			   &Dump_GYM_Dir[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "Screen Shot path", Rom_Dir,
			   &ScrShot_Dir[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "Patch path", Rom_Dir, &Patch_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "IPS Patch path", Rom_Dir, &IPS_Dir[0],
			   1024, Conf_File);

  GetPrivateProfileString ("General", "Genesis Bios", Rom_Dir,
			   &Genesis_Bios[0], 1024, Conf_File);

  //~ GetPrivateProfileString ("General", "USA CD Bios", Rom_Dir, &US_CD_Bios[0],
			   //~ 1024, Conf_File);
  //~ GetPrivateProfileString ("General", "EUROPE CD Bios", Rom_Dir,
			   //~ &EU_CD_Bios[0], 1024, Conf_File);
  //~ GetPrivateProfileString ("General", "JAPAN CD Bios", Rom_Dir,
			   //~ &JA_CD_Bios[0], 1024, Conf_File);

  //~ GetPrivateProfileString ("General", "32X 68000 Bios", Rom_Dir,
			   //~ &_32X_Genesis_Bios[0], 1024, Conf_File);
  //~ GetPrivateProfileString ("General", "32X Master SH2 Bios", Rom_Dir,
			   //~ &_32X_Master_Bios[0], 1024, Conf_File);
  //~ GetPrivateProfileString ("General", "32X Slave SH2 Bios", Rom_Dir,
			   //~ &_32X_Slave_Bios[0], 1024, Conf_File);

  GetPrivateProfileString ("General", "Rom 1", "", &Recent_Rom[0][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 2", "", &Recent_Rom[1][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 3", "", &Recent_Rom[2][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 4", "", &Recent_Rom[3][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 5", "", &Recent_Rom[4][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 6", "", &Recent_Rom[5][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 7", "", &Recent_Rom[6][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 8", "", &Recent_Rom[7][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 9", "", &Recent_Rom[8][0], 1024,
			   Conf_File);

  Current_State =
    GetPrivateProfileInt ("General", "State Number", 0, Conf_File);
  Language = GetPrivateProfileInt ("General", "Language", 0, Conf_File);
  Window_Pos.x = GetPrivateProfileInt ("General", "Window X", 0, Conf_File);
  Window_Pos.y = GetPrivateProfileInt ("General", "Window Y", 0, Conf_File);
  Intro_Style = GetPrivateProfileInt ("General", "Intro Style", 0, Conf_File);
  Effect_Color =
    GetPrivateProfileInt ("General", "Free Mode Color", 7, Conf_File);
  Sleep_Time =
    GetPrivateProfileInt ("General", "Allow Idle", 0, Conf_File) & 1;

  if (GetPrivateProfileInt ("Graphics", "Force 555", 0, Conf_File))
    Mode_555 = 3;
  else if (GetPrivateProfileInt ("Graphics", "Force 565", 0, Conf_File))
    Mode_555 = 2;
  else
    Mode_555 = 0;

  RMax_Level = GetPrivateProfileInt ("Graphics", "Red Max", 255, Conf_File);
  GMax_Level = GetPrivateProfileInt ("Graphics", "Green Max", 255, Conf_File);
  BMax_Level = GetPrivateProfileInt ("Graphics", "Blue Max", 255, Conf_File);
  Contrast_Level =
    GetPrivateProfileInt ("Graphics", "Contrast", 100, Conf_File);
  Brightness_Level =
    GetPrivateProfileInt ("Graphics", "Brightness", 100, Conf_File);
  Greyscale = GetPrivateProfileInt ("Graphics", "Greyscale", 0, Conf_File);
  Invert_Color = GetPrivateProfileInt ("Graphics", "Invert", 0, Conf_File);

  Recalculate_Palettes ();

  FS_VSync =
    GetPrivateProfileInt ("Graphics", "Full Screen VSync", 0, Conf_File);
  W_VSync = GetPrivateProfileInt ("Graphics", "Windows VSync", 0, Conf_File);
  Full_Screen =
    GetPrivateProfileInt ("Graphics", "Full Screen", 0, Conf_File);
  Render_W =
    GetPrivateProfileInt ("Graphics", "Render Windowed", 0, Conf_File);
  Render_FS =
    GetPrivateProfileInt ("Graphics", "Render Fullscreen", 1, Conf_File);

  Set_Render (Full_Screen, -1, 1);

  Stretch = GetPrivateProfileInt ("Graphics", "Stretch", 0, Conf_File);
  Blit_Soft =
    GetPrivateProfileInt ("Graphics", "Software Blit", 0, Conf_File);
  Sprite_Over =
    GetPrivateProfileInt ("Graphics", "Sprite limit", 1, Conf_File);
  Frame_Skip = GetPrivateProfileInt ("Graphics", "Frame skip", -1, Conf_File);

  Sound_Rate = GetPrivateProfileInt ("Sound", "Rate", 22050, Conf_File);
  Sound_Stereo = GetPrivateProfileInt ("Sound", "Stereo", 1, Conf_File);

  if (GetPrivateProfileInt ("Sound", "Z80 State", 1, Conf_File))
    Z80_State |= 1;
  else
    Z80_State &= ~1;

  new_val = GetPrivateProfileInt ("Sound", "State", 0, Conf_File);
#ifdef NO_SOUND
  Sound_Enable=new_val=0;
#else
  if (new_val != Sound_Enable)
    {
      if (Change_Sound ())
	{
	  YM2612_Enable =
	    GetPrivateProfileInt ("Sound", "YM2612 State", 1, Conf_File);
	  PSG_Enable =
	    GetPrivateProfileInt ("Sound", "PSG State", 1, Conf_File);
	  DAC_Enable =
	    GetPrivateProfileInt ("Sound", "DAC State", 1, Conf_File);
#ifdef SEGA_CD

	  PCM_Enable =
	    GetPrivateProfileInt ("Sound", "PCM State", 1, Conf_File);
#endif
#ifdef SEGA_32X
	  PWM_Enable =
	    GetPrivateProfileInt ("Sound", "PWM State", 1, Conf_File);
#endif

#ifdef SEGA_CD
	  CDDA_Enable =
	    GetPrivateProfileInt ("Sound", "CDDA State", 1, Conf_File);
#endif
	}
    }
  else
#endif
    {
      YM2612_Enable =
	GetPrivateProfileInt ("Sound", "YM2612 State", 1, Conf_File);
      PSG_Enable = GetPrivateProfileInt ("Sound", "PSG State", 1, Conf_File);
      DAC_Enable = GetPrivateProfileInt ("Sound", "DAC State", 1, Conf_File);
#ifdef SEGA_CD

      PCM_Enable = GetPrivateProfileInt ("Sound", "PCM State", 1, Conf_File);
#endif
#ifdef SEGA_32X
      PWM_Enable = GetPrivateProfileInt ("Sound", "PWM State", 1, Conf_File);
#endif

#ifdef SEGA_CD
      CDDA_Enable =	GetPrivateProfileInt ("Sound", "CDDA State", 1, Conf_File);
#endif
    }

  YM2612_Improv =
    GetPrivateProfileInt ("Sound", "YM2612 Improvement", 0, Conf_File);
  DAC_Improv =
    GetPrivateProfileInt ("Sound", "DAC Improvement", 0, Conf_File);
  PSG_Improv =
    GetPrivateProfileInt ("Sound", "PSG Improvement", 0, Conf_File);

  Country = GetPrivateProfileInt ("CPU", "Country", -1, Conf_File);
  Country_Order[0] =
    GetPrivateProfileInt ("CPU", "Prefered Country 1", 0, Conf_File);
  Country_Order[1] =
    GetPrivateProfileInt ("CPU", "Prefered Country 2", 1, Conf_File);
  Country_Order[2] =
    GetPrivateProfileInt ("CPU", "Prefered Country 3", 2, Conf_File);

#ifdef SEGA_CD
  SegaCD_Accurate =
    GetPrivateProfileInt ("CPU",
			  "Perfect synchro between main and sub CPU (Sega CD)",
			  0, Conf_File);
#endif

#ifdef SEGA_32X
  MSH2_Speed = GetPrivateProfileInt ("CPU", "Main SH2 Speed", 100, Conf_File);
  SSH2_Speed =
    GetPrivateProfileInt ("CPU", "Slave SH2 Speed", 100, Conf_File);

  if (MSH2_Speed < 0)
    MSH2_Speed = 0;
  if (SSH2_Speed < 0)
    SSH2_Speed = 0;
#endif

  Check_Country_Order ();

  Fast_Blur = GetPrivateProfileInt ("Options", "Fast Blur", 0, Conf_File);
  if (!Show_FPS)
   Show_FPS = GetPrivateProfileInt ("Options", "FPS", 0, Conf_File);
  FPS_Style = GetPrivateProfileInt ("Options", "FPS Style", 0, Conf_File);
  Show_Message = GetPrivateProfileInt ("Options", "Message", 1, Conf_File);
  Message_Style =
    GetPrivateProfileInt ("Options", "Message Style", 0, Conf_File);
  Show_LED = GetPrivateProfileInt ("Options", "LED", 1, Conf_File);
  Auto_Fix_CS =
    GetPrivateProfileInt ("Options", "Auto Fix Checksum", 0, Conf_File);
  Auto_Pause = GetPrivateProfileInt ("Options", "Auto Pause", 0, Conf_File);

#ifdef SEGA_CD
  CUR_DEV = GetPrivateProfileInt ("Options", "CD Drive", 0, Conf_File);
#endif

  BRAM_Ex_Size =
    GetPrivateProfileInt ("Options", "Ram Cart Size", 3, Conf_File);

  if (BRAM_Ex_Size == -1)
    {
      BRAM_Ex_State &= 1;
      BRAM_Ex_Size = 0;
    }
  else
    BRAM_Ex_State |= 0x100;

  GetPrivateProfileString ("Options", "GCOffline path", "GCOffline.chm",
			   CGOffline_Path, 1024, Conf_File);
  GetPrivateProfileString ("Options", "Gens manual path", "manual.exe",
			   Manual_Path, 1024, Conf_File);

  Controller_1_Type = GetPrivateProfileInt ("Input", "P1.Type", 0, Conf_File);
  Keys_Def[0].Up = GetPrivateProfileInt ("Input", "P1.Up", DIK_UP, Conf_File);
  Keys_Def[0].Down =
    GetPrivateProfileInt ("Input", "P1.Down", DIK_DOWN, Conf_File);
  Keys_Def[0].Left =
    GetPrivateProfileInt ("Input", "P1.Left", DIK_LEFT, Conf_File);
  Keys_Def[0].Right =
    GetPrivateProfileInt ("Input", "P1.Right", DIK_RIGHT, Conf_File);
  Keys_Def[0].Start =
    GetPrivateProfileInt ("Input", "P1.Start", DIK_RETURN, Conf_File);
  Keys_Def[0].A = GetPrivateProfileInt ("Input", "P1.A", DIK_A, Conf_File);
  Keys_Def[0].B = GetPrivateProfileInt ("Input", "P1.B", DIK_S, Conf_File);
  Keys_Def[0].C = GetPrivateProfileInt ("Input", "P1.C", DIK_D, Conf_File);
  Keys_Def[0].Mode =
    GetPrivateProfileInt ("Input", "P1.Mode", DIK_RSHIFT, Conf_File);
  Keys_Def[0].X = GetPrivateProfileInt ("Input", "P1.X", DIK_Z, Conf_File);
  Keys_Def[0].Y = GetPrivateProfileInt ("Input", "P1.Y", DIK_X, Conf_File);
  Keys_Def[0].Z = GetPrivateProfileInt ("Input", "P1.Z", DIK_C, Conf_File);

  Controller_1B_Type =
    GetPrivateProfileInt ("Input", "P1B.Type", 0, Conf_File);
  Keys_Def[2].Up = GetPrivateProfileInt ("Input", "P1B.Up", 0, Conf_File);
  Keys_Def[2].Down = GetPrivateProfileInt ("Input", "P1B.Down", 0, Conf_File);
  Keys_Def[2].Left = GetPrivateProfileInt ("Input", "P1B.Left", 0, Conf_File);
  Keys_Def[2].Right =
    GetPrivateProfileInt ("Input", "P1B.Right", 0, Conf_File);
  Keys_Def[2].Start =
    GetPrivateProfileInt ("Input", "P1B.Start", 0, Conf_File);
  Keys_Def[2].A = GetPrivateProfileInt ("Input", "P1B.A", 0, Conf_File);
  Keys_Def[2].B = GetPrivateProfileInt ("Input", "P1B.B", 0, Conf_File);
  Keys_Def[2].C = GetPrivateProfileInt ("Input", "P1B.C", 0, Conf_File);
  Keys_Def[2].Mode = GetPrivateProfileInt ("Input", "P1B.Mode", 0, Conf_File);
  Keys_Def[2].X = GetPrivateProfileInt ("Input", "P1B.X", 0, Conf_File);
  Keys_Def[2].Y = GetPrivateProfileInt ("Input", "P1B.Y", 0, Conf_File);
  Keys_Def[2].Z = GetPrivateProfileInt ("Input", "P1B.Z", 0, Conf_File);

  Controller_1C_Type =
    GetPrivateProfileInt ("Input", "P1C.Type", 0, Conf_File);
  Keys_Def[3].Up = GetPrivateProfileInt ("Input", "P1C.Up", 0, Conf_File);
  Keys_Def[3].Down = GetPrivateProfileInt ("Input", "P1C.Down", 0, Conf_File);
  Keys_Def[3].Left = GetPrivateProfileInt ("Input", "P1C.Left", 0, Conf_File);
  Keys_Def[3].Right =
    GetPrivateProfileInt ("Input", "P1C.Right", 0, Conf_File);
  Keys_Def[3].Start =
    GetPrivateProfileInt ("Input", "P1C.Start", 0, Conf_File);
  Keys_Def[3].A = GetPrivateProfileInt ("Input", "P1C.A", 0, Conf_File);
  Keys_Def[3].B = GetPrivateProfileInt ("Input", "P1C.B", 0, Conf_File);
  Keys_Def[3].C = GetPrivateProfileInt ("Input", "P1C.C", 0, Conf_File);
  Keys_Def[3].Mode = GetPrivateProfileInt ("Input", "P1C.Mode", 0, Conf_File);
  Keys_Def[3].X = GetPrivateProfileInt ("Input", "P1C.X", 0, Conf_File);
  Keys_Def[3].Y = GetPrivateProfileInt ("Input", "P1C.Y", 0, Conf_File);
  Keys_Def[3].Z = GetPrivateProfileInt ("Input", "P1C.Z", 0, Conf_File);

  Controller_1D_Type =
    GetPrivateProfileInt ("Input", "P1D.Type", 0, Conf_File);
  Keys_Def[4].Up = GetPrivateProfileInt ("Input", "P1D.Up", 0, Conf_File);
  Keys_Def[4].Down = GetPrivateProfileInt ("Input", "P1D.Down", 0, Conf_File);
  Keys_Def[4].Left = GetPrivateProfileInt ("Input", "P1D.Left", 0, Conf_File);
  Keys_Def[4].Right =
    GetPrivateProfileInt ("Input", "P1D.Right", 0, Conf_File);
  Keys_Def[4].Start =
    GetPrivateProfileInt ("Input", "P1D.Start", 0, Conf_File);
  Keys_Def[4].A = GetPrivateProfileInt ("Input", "P1D.A", 0, Conf_File);
  Keys_Def[4].B = GetPrivateProfileInt ("Input", "P1D.B", 0, Conf_File);
  Keys_Def[4].C = GetPrivateProfileInt ("Input", "P1D.C", 0, Conf_File);
  Keys_Def[4].Mode = GetPrivateProfileInt ("Input", "P1D.Mode", 0, Conf_File);
  Keys_Def[4].X = GetPrivateProfileInt ("Input", "P1D.X", 0, Conf_File);
  Keys_Def[4].Y = GetPrivateProfileInt ("Input", "P1D.Y", 0, Conf_File);
  Keys_Def[4].Z = GetPrivateProfileInt ("Input", "P1D.Z", 0, Conf_File);

  Controller_2_Type = GetPrivateProfileInt ("Input", "P2.Type", 0, Conf_File);
  Keys_Def[1].Up = GetPrivateProfileInt ("Input", "P2.Up", DIK_Y, Conf_File);
  Keys_Def[1].Down =
    GetPrivateProfileInt ("Input", "P2.Down", DIK_H, Conf_File);
  Keys_Def[1].Left =
    GetPrivateProfileInt ("Input", "P2.Left", DIK_G, Conf_File);
  Keys_Def[1].Right =
    GetPrivateProfileInt ("Input", "P2.Right", DIK_J, Conf_File);
  Keys_Def[1].Start =
    GetPrivateProfileInt ("Input", "P2.Start", DIK_U, Conf_File);
  Keys_Def[1].A = GetPrivateProfileInt ("Input", "P2.A", DIK_K, Conf_File);
  Keys_Def[1].B = GetPrivateProfileInt ("Input", "P2.B", DIK_L, Conf_File);
  Keys_Def[1].C = GetPrivateProfileInt ("Input", "P2.C", DIK_M, Conf_File);
  Keys_Def[1].Mode =
    GetPrivateProfileInt ("Input", "P2.Mode", DIK_T, Conf_File);
  Keys_Def[1].X = GetPrivateProfileInt ("Input", "P2.X", DIK_I, Conf_File);
  Keys_Def[1].Y = GetPrivateProfileInt ("Input", "P2.Y", DIK_O, Conf_File);
  Keys_Def[1].Z = GetPrivateProfileInt ("Input", "P2.Z", DIK_P, Conf_File);

  Controller_2B_Type =
    GetPrivateProfileInt ("Input", "P2B.Type", 0, Conf_File);
  Keys_Def[5].Up = GetPrivateProfileInt ("Input", "P2B.Up", 0, Conf_File);
  Keys_Def[5].Down = GetPrivateProfileInt ("Input", "P2B.Down", 0, Conf_File);
  Keys_Def[5].Left = GetPrivateProfileInt ("Input", "P2B.Left", 0, Conf_File);
  Keys_Def[5].Right =
    GetPrivateProfileInt ("Input", "P2B.Right", 0, Conf_File);
  Keys_Def[5].Start =
    GetPrivateProfileInt ("Input", "P2B.Start", 0, Conf_File);
  Keys_Def[5].A = GetPrivateProfileInt ("Input", "P2B.A", 0, Conf_File);
  Keys_Def[5].B = GetPrivateProfileInt ("Input", "P2B.B", 0, Conf_File);
  Keys_Def[5].C = GetPrivateProfileInt ("Input", "P2B.C", 0, Conf_File);
  Keys_Def[5].Mode = GetPrivateProfileInt ("Input", "P2B.Mode", 0, Conf_File);
  Keys_Def[5].X = GetPrivateProfileInt ("Input", "P2B.X", 0, Conf_File);
  Keys_Def[5].Y = GetPrivateProfileInt ("Input", "P2B.Y", 0, Conf_File);
  Keys_Def[5].Z = GetPrivateProfileInt ("Input", "P2B.Z", 0, Conf_File);

  Controller_2C_Type =
    GetPrivateProfileInt ("Input", "P2C.Type", 0, Conf_File);
  Keys_Def[6].Up = GetPrivateProfileInt ("Input", "P2C.Up", 0, Conf_File);
  Keys_Def[6].Down = GetPrivateProfileInt ("Input", "P2C.Down", 0, Conf_File);
  Keys_Def[6].Left = GetPrivateProfileInt ("Input", "P2C.Left", 0, Conf_File);
  Keys_Def[6].Right =
    GetPrivateProfileInt ("Input", "P2C.Right", 0, Conf_File);
  Keys_Def[6].Start =
    GetPrivateProfileInt ("Input", "P2C.Start", 0, Conf_File);
  Keys_Def[6].A = GetPrivateProfileInt ("Input", "P2C.A", 0, Conf_File);
  Keys_Def[6].B = GetPrivateProfileInt ("Input", "P2C.B", 0, Conf_File);
  Keys_Def[6].C = GetPrivateProfileInt ("Input", "P2C.C", 0, Conf_File);
  Keys_Def[6].Mode = GetPrivateProfileInt ("Input", "P2C.Mode", 0, Conf_File);
  Keys_Def[6].X = GetPrivateProfileInt ("Input", "P2C.X", 0, Conf_File);
  Keys_Def[6].Y = GetPrivateProfileInt ("Input", "P2C.Y", 0, Conf_File);
  Keys_Def[6].Z = GetPrivateProfileInt ("Input", "P2C.Z", 0, Conf_File);

  Controller_2D_Type =
    GetPrivateProfileInt ("Input", "P2D.Type", 0, Conf_File);
  Keys_Def[7].Up = GetPrivateProfileInt ("Input", "P2D.Up", 0, Conf_File);
  Keys_Def[7].Down = GetPrivateProfileInt ("Input", "P2D.Down", 0, Conf_File);
  Keys_Def[7].Left = GetPrivateProfileInt ("Input", "P2D.Left", 0, Conf_File);
  Keys_Def[7].Right =
    GetPrivateProfileInt ("Input", "P2D.Right", 0, Conf_File);
  Keys_Def[7].Start =
    GetPrivateProfileInt ("Input", "P2D.Start", 0, Conf_File);
  Keys_Def[7].A = GetPrivateProfileInt ("Input", "P2D.A", 0, Conf_File);
  Keys_Def[7].B = GetPrivateProfileInt ("Input", "P2D.B", 0, Conf_File);
  Keys_Def[7].C = GetPrivateProfileInt ("Input", "P2D.C", 0, Conf_File);
  Keys_Def[7].Mode = GetPrivateProfileInt ("Input", "P2D.Mode", 0, Conf_File);
  Keys_Def[7].X = GetPrivateProfileInt ("Input", "P2D.X", 0, Conf_File);
  Keys_Def[7].Y = GetPrivateProfileInt ("Input", "P2D.Y", 0, Conf_File);
  Keys_Def[7].Z = GetPrivateProfileInt ("Input", "P2D.Z", 0, Conf_File);

  Make_IO_Table ();
  return 1;
}


int
Load_As_Config (void *Game_Active)
{
/*
  char *filename;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Load Config", GTK_FILE_CHOOSER_ACTION_OPEN);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addCfgFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (strlen (filename))
    {
      Load_Config (filename, Game_Active);
      strcpy (Str_Tmp, "config loaded from ");
      strcat (Str_Tmp, filename);
      Put_Info (Str_Tmp, 2000);
      g_free (filename);
      return 1;
    }
*/
  return 0;
}


static int GetSRAMSize(void) {
	
	int i, size_to_save;
	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0))
		i--;
	if (i < 0)
		return 0;
	i++;
	size_to_save = 1;
	while (i > size_to_save)
		size_to_save <<= 1;
	
	printf("Calced %i\n", size_to_save);
	
	if (GLCurGame) {
		int GLSize = GL_CalcSRAMSize(GLCurGame->SRAMSize);
		
		if (size_to_save < GLSize)
			size_to_save = GLSize;
	}
	return size_to_save;
}

/*
	Returns:
	0 if nothing to save
	1 if only even bytes have data
	2 if only odd bytes have data
	3 if both even and odd bytes have data
*/
static int GetSRAMColumns(int size) {
	if (GLCurGame) {
		switch(GLCurGame->SRAM) {
		case SAV_NONE: return 0;
		case SAV_ODD_BYTES_SRAM: return 2;
		case SAV_EVEN_BYTES_SRAM: return 1;
		case SAV_WORD_SRAM: return 3;
		case SAV_EEPROM: return 3;
		default:
			;
		};
	}
	
	int i, cols = 0;
	for(i = 0; i < size; i++) {
		if (SRAM[i]) {
			if (i & 1) {
				//Odd byte
				cols |= 2;
			} else {
				//Even
				cols |= 1;
			}
		}
	}
	return cols;
}

static void SRAMSqueeze(const unsigned char *src, unsigned char *dst, int size) {
	int i;
	for(i = 0; i < size; i++) {
		*dst++ = *src;
		src += 2;
	}
}

static void SRAMStretch(const unsigned char *src, unsigned char *dst, int size) {
	int i;
	for(i = 0; i < size; i++) {
		*dst = *src++;
		dst += 2;
	}
}

static int FileSizeBlocks(const char *name) {
	FILE *f = fopen(name, "rb");
	if (f == NULL) {
		return 0;
	}
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fclose(f);
	printf("Len %i\n", len);
	return (len + 511) / 512;
}
/*
static int FileSizeBlocks(const char *name) {
	struct stat dstat;
	memset(&dstat, 0, sizeof(dstat));
	if (stat(name, &dstat) == -1) {
		printf("Couldn't stat file \"%s\"\n", name);
		return 0;
	}
	
	return (dstat.st_size + 511) / 512;
}*/

static char SaveName[2000];
static unsigned char FileBuff[66*1024];
char FriendlySaveDest[1000];
int Blocks_Required;
int Destination_Free_Space;
static uint8_t *VMUData;
static int VMUDataSize;


static void GetSaveFileName(void) {
	if (SAVE_TO_PC) {
		strcpy (SaveName, "/pc/home/alex/Emu/Genesis/GENS4ALL_");
		strcpy(FriendlySaveDest, "PC");
	} else {
		maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_MEMCARD);
		if (vmu) {
			sprintf(SaveName, "/vmu/%c%i/GENS4ALL_", 'a' + vmu->port, vmu->unit);
			sprintf(FriendlySaveDest, "VMU %c%i", 'A' + vmu->port, vmu->unit);
			Destination_Free_Space = vmufs_free_blocks(vmu);
		} else {
			strcpy(SaveName, "/ram/GENS4ALL_");
			strcpy(FriendlySaveDest, "Temporary RAM");
			Destination_Free_Space = 100;
		}
	}
	
	//If using the loader and it has a save name, use that
	if (GLCurGame && memcmp(GLCurGame->SaveExtension, "000", 3) != 0) {
		int len = strlen(SaveName);
		memcpy(SaveName + len, GLCurGame->SaveExtension, 3);
		SaveName[len+4] = 0;
	} else {
		//Hash the ROM serial number for the end of the filename
		unsigned int hash = 5381, i;
		for(i = 0; i < sizeof(Game->Version); i++) {
			hash = (hash <<5) + hash + Game->Version[i];
		}
		hash ^= hash>>16;
		
		//Convert to letters
		char mininame[4] = {0,0,0,0};
		for(i = 0; i < 3; i++) { 
			mininame[i] = hash % 52;
			hash /= 52;
			
			mininame[i] = (mininame[i] > 26) ? ('a' + mininame[i] - 26) : (mininame[i] + 'A');
		}
		//~ char mininame[] = {Rom_Name[0],Rom_Name[1],Rom_Name[2], 0};
		strcat (SaveName, mininame);
	}
}

int Has_SRAM_Changed(void) {
	if (GLCurGame && GLCurGame->SRAM == SAV_NONE)
		return 0;
	else
		return memcmp(SRAM, OrigSRAM, 64*1024) != 0;
}

typedef struct {
	char version;
	char type;
	unsigned short size;
	unsigned char data[];
} SaveHeader;

int Load_SRAM (void)
{
	FILE *SRAM_File = 0;
	
	memset (SRAM, 0, 64 * 1024);
	memset (OrigSRAM, 0, 64 * 1024);
	
	GetSaveFileName();
	
	printf("***LOADING SRAM***\n%s\n", SaveName);
	if ((SRAM_File = fopen (SaveName, "rb")) == 0)
		return 0;
	fread (FileBuff, 64 * 1024, 1, SRAM_File);
	
	if (!feof(SRAM_File)) {
		//Incomplete read? Failure.
		fclose (SRAM_File);
		return 0;
	}
	fclose (SRAM_File);
	
	vmu_pkg_t vmupkg;
	if (vmu_pkg_parse(FileBuff, &vmupkg)) {
		//CRC error
		return 0;
	}
	
	SaveHeader *shead = (void*)vmupkg.data;
	
	if (shead->version == 0) {
		if (shead->size > sizeof(SRAM))
			return 0;
		
		if (shead->type == 1)
			SRAMStretch(shead->data, SRAM, shead->size);
		else if (shead->type == 2)
			SRAMStretch(shead->data, SRAM+1, shead->size);
		else if (shead->type == 3)
			memcpy(SRAM, shead->data, shead->size);
	} else {
		//TODO error unknown version
		return 0;
	}
	
	memcpy(OrigSRAM, SRAM, 64*1024);
	
	strcpy (Str_Tmp, "SRAM loaded from ");
	strcat (Str_Tmp, SaveName);
	Put_Info (Str_Tmp, 2000);
	return 1;
}


int Prepare_Save_SRAM (void)
{
	assert(VMUData == NULL);
	Blocks_Required = 0;
	Destination_Free_Space = 0;
	
	//~ maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_MEMCARD);
	//~ if (vmu == NULL)
		//~ return 0;
	
	//~ printf("Got VMU at %c%i\n", 'a' + vmu->port, vmu->unit);
	
	int size_to_save = GetSRAMSize();
	printf("***PREPARING SAVE SRAM***\n%s (%i byte SRAM)\n", SaveName, size_to_save);
	
	GetSaveFileName();
		
	SaveHeader *shead = (void*)FileBuff;
	
	//Squeeze SRAM and add header
	int cols = GetSRAMColumns(size_to_save);
	
	if (cols == 0) {
		return 0;	//Nothing to save
	} else if (cols == 1) {
		size_to_save /= 2;
		SRAMSqueeze(SRAM, shead->data, size_to_save);
	} else if (cols == 2) {
		size_to_save /= 2;
		SRAMSqueeze(SRAM+1, shead->data, size_to_save);
	} else {
		memcpy(shead->data, SRAM, size_to_save);
	}
	
	if (GLCurGame && size_to_save < GL_CalcSRAMSize(GLCurGame->SRAMSize)) {
		size_to_save = GL_CalcSRAMSize(GLCurGame->SRAMSize);
	}
	
	shead->version = 0;
	shead->type = cols;
	shead->size = size_to_save;
	
	printf("SRAM size after squeeze: %i\n", size_to_save);
	
	//Make header
	vmu_pkg_t vmupkg;
	//~ strncpy(vmupkg.app_id, SaveName, sizeof(vmupkg.app_id)-1);
	strcpy(vmupkg.app_id, "Gens4All");
	//~ strcpy(vmupkg.desc_long, "Gens4All");	//Game->Rom_Name_W
	if (GLCurGame) {
		strncpy(vmupkg.desc_long, GetCurGameSaveTitle(), sizeof(vmupkg.desc_long)-1);
		vmupkg.desc_long[sizeof(vmupkg.desc_long)-1] = 0;
		strncpy(vmupkg.desc_short, GetCurGameSaveVMUTitle(), sizeof(vmupkg.desc_short)-1);
		vmupkg.desc_short[sizeof(vmupkg.desc_short)-1] = 0;
	} else {
		strncpy(vmupkg.desc_long, Game->Rom_Name_W, sizeof(vmupkg.desc_long)-1);
		vmupkg.desc_long[sizeof(vmupkg.desc_long)-1] = 0;
		strncpy(vmupkg.desc_short, Game->Rom_Name_W, sizeof(vmupkg.desc_short)-1);
		vmupkg.desc_short[sizeof(vmupkg.desc_short)-1] = 0;
	}
	vmupkg.icon_cnt = 0;
	vmupkg.icon_anim_speed = 0;
	vmupkg.eyecatch_type = VMUPKG_EC_NONE;
	vmupkg.data_len = size_to_save+3;
	memset(vmupkg.icon_pal, 0, sizeof(vmupkg.icon_pal));
	vmupkg.icon_data = 0;
	vmupkg.eyecatch_data = 0;
	vmupkg.data = FileBuff;
	
	if (vmu_pkg_build(&vmupkg, &VMUData, &VMUDataSize)) {
		assert(0);
	}
	/*
	vmu_timestamp_t replacetime;
	DCTimeConvert(rtc_unix_secs(), &replacetime);
	vmufs_file_write(
	*/
	//Check for space required
	Blocks_Required = (VMUDataSize + 511) / 512 - FileSizeBlocks(SaveName);
	//If the SRAM zeroes out so it shrinks, don't shrink VMU file
	if (Blocks_Required < 0)
		Blocks_Required = 0;
	
	printf("Calc %i byte SRAM\n",  GetSRAMSize());
	printf("%i block size, %i extra blocks needed, %i blocks free\n", (VMUDataSize + 511) / 512, Blocks_Required, Destination_Free_Space);
	printf("VMU file size %i\n", VMUDataSize);
	
	return 1;
}

void Abort_Save_SRAM(void) {
	if (VMUData != NULL) {
		free(VMUData);
		VMUData = NULL;
	}
	VMUDataSize = 0;
}

int Commit_Save_SRAM(void) {
	if (VMUData == NULL)
		return 0;
	
	FILE *SRAM_File = 0;
	
	if ((SRAM_File = fopen (SaveName, "wb")) == 0)
		goto failure;

	printf("***COMMIT SAVE SRAM***\n");
	//~ fwrite (SRAM, size_to_save, 1, SRAM_File);
	watchdog_pause();
	fwrite (VMUData, VMUDataSize, 1, SRAM_File);
	fclose (SRAM_File);
	watchdog_resume();

	strcpy (Str_Tmp, "SRAM saved in ");
	strcat (Str_Tmp, SaveName);
	Put_Info (Str_Tmp, 2000);
	
	Abort_Save_SRAM();
	memcpy(OrigSRAM, SRAM, 64*1024);
	return 1;
failure:
	Abort_Save_SRAM();
	return 0;
}

int Save_SRAM (void)
{
	if (Has_SRAM_Changed() && Prepare_Save_SRAM()) {
		return Commit_Save_SRAM();
	}
	return 0;
}
int
Save_SRAM_FS (void)
{
	FILE *SRAM_File = 0;
	int size_to_save, i;
	char Name[2048];

	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0))
		i--;

	if (i < 0)
		return 0;

	i++;

	size_to_save = 1;
	while (i > size_to_save)
		size_to_save <<= 1;

	strcpy (Name, SRAM_Dir);
	strcat (Name, Rom_Name);
	strcat (Name, ".srm");

	printf("***SAVING SRAM***\n%s (%i bytes)\n", Name, size_to_save);
	if ((SRAM_File = fopen (Name, "wb")) == 0)
		return 0;

	fwrite (SRAM, size_to_save, 1, SRAM_File);
	fclose (SRAM_File);

	strcpy (Str_Tmp, "SRAM saved in ");
	strcat (Str_Tmp, Name);
	Put_Info (Str_Tmp, 2000);

	return 1;
}

void
S_Format_BRAM (unsigned char *buf)
{
  memset (buf, 0x5F, 11);

  buf[0x0F] = 0x40;

  buf[0x11] = 0x7D;
  buf[0x13] = 0x7D;
  buf[0x15] = 0x7D;
  buf[0x17] = 0x7D;

  sprintf ((char *) &buf[0x20], "SEGA CD ROM");
  sprintf ((char *) &buf[0x30], "RAM CARTRIDGE");

  buf[0x24] = 0x5F;
  buf[0x27] = 0x5F;

  buf[0x2C] = 0x01;

  buf[0x33] = 0x5F;
  buf[0x3D] = 0x5F;
  buf[0x3E] = 0x5F;
  buf[0x3F] = 0x5F;
}


#ifdef SEGA_CD
void Format_Backup_Ram (void)
{
	memset (Ram_Backup, 0, 8 * 1024);
	S_Format_BRAM (&Ram_Backup[0x1FC0]);
	memset (Ram_Backup_Ex, 0, 64 * 1024);
}

int Load_BRAM (void)
{
	FILE *BRAM_File = 0;
	char Name[2048];
	
	Format_Backup_Ram ();
	
	strcpy (Name, BRAM_Dir);
	strcat (Name, Rom_Name);
	strcat (Name, ".brm");
	
	if ((BRAM_File = fopen (Name, "rb")) == 0)
	return 0;
	
	fread (Ram_Backup, 8 * 1024, 1, BRAM_File);
	fread (Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose (BRAM_File);
	
	strcpy (Str_Tmp, "BRAM loaded from ");
	strcat (Str_Tmp, Name);
	Put_Info (Str_Tmp, 2000);
	return 1;
}

int Save_BRAM (void)
{
	FILE *BRAM_File = 0;
	char Name[2048];
	
	strcpy (Name, BRAM_Dir);
	strcat (Name, Rom_Name);
	strcat (Name, ".brm");
	
	if ((BRAM_File = fopen (Name, "wb")) == 0)
		return 0;
	
	fwrite (Ram_Backup, 8 * 1024, 1, BRAM_File);
	fwrite (Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose (BRAM_File);
	
	strcpy (Str_Tmp, "BRAM saved in ");
	strcat (Str_Tmp, Name);
	Put_Info (Str_Tmp, 2000);
	return 1;
}
#endif
